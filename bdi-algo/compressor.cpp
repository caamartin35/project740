#include "compressor.h"

using std::cout;
using std::endl;
using std::list;
using std::vector;

Compressor::Compressor(int size, int ways, int block_size) {
  // configure the cache dimensions
  this->size = size;
  this->ways = ways;
  this->block_size = block_size;
  this->sets = size / ways / block_size;

  // compute the address masks
  set_bits = log2(sets);
  bib_bits = log2(block_size);
  tag_bits = (sizeof(pointer_t) * BITS_IN_BYTE) - set_bits - bib_bits;

  // allocate cache memory
  tag_store.resize(sets);
  for (int i = 0; i < tag_store.size(); i++) {
    // twice as many tags
    for (int j = 0; j < 2 * ways; j++) {
      Tag tag;
      tag_store[i].push_back(tag);
    }
  }
  data_store.resize(sets);
  for (int i = 0; i < data_store.size(); i++) {
    data_store[i].resize(block_size * ways);
  }

  // initialize counters
  requests = 0;
  hits = 0;
  misses = 0;
  evictions = 0;

  // more stats
  used = 0;
}

void Compressor::Cycle() {
  // any cycling logic (per mem operation)
  // maintain stats, etc.
}

bool Compressor::Load(pointer_t address, size_t size, data_t data) {
  pointer_t index = getSet(address);
  pointer_t tag_value = getTag(address);
  list<Tag>* tags = &tag_store[index];
  Tag* tag = contains(tags, tag_value);
  if (!tag) {
    requests++;
    misses++;
    insert(address, size, data);
    return false;
  } else {
    requests++;
    hits++;
    touchTag(tags, *tag);
    tag->age = 0;
    return true;
  }
}

bool Compressor::Store(pointer_t address, size_t size, data_t data) {
  int index = getSet(address);
  list<Tag>* tags = &tag_store[index];
  pointer_t needle = getTag(address);
  Tag *tag = contains(tags, needle);
  if (!tag) {
    requests++;
    misses++;
    insert(address, size, data);
    return false;
  } else {
    requests++;
    hits++;
    insert(address, size, data);
    return true;
  }
}

BaseDeltaStats Compressor::Stats() const {
  BaseDeltaStats stats;
  stats.requests = requests;
  stats.hits = hits;
  stats.misses = misses;
  stats.evictions = evictions;
  stats.used_space = used;
  // compute the rest
  stats.used_blocks = 0;
  stats.zero_blocks = 0;
  stats.rep_blocks = 0;
  stats.medium_blocks = 0;
  stats.large_blocks = 0;
  // iterate
  for (int i = 0; i < tag_store.size(); i++) {
    const list<Tag>& tags = tag_store[i];
    list<Tag>::const_iterator it;
    for (it = tags.begin(); it != tags.end(); ++it) {
      const Tag& tag = *it;
      if (!tag.valid) continue;
      stats.used_blocks++;
      switch (tag.mode) {
        case ZEROS:
          stats.zero_blocks++; break;
        case REP_VALUES:
          stats.rep_blocks++; break;
        case BASE4_DELTA2:
        case BASE8_DELTA4:
        case NO_COMPRESS:
          stats.large_blocks++; break;
        default:
          stats.medium_blocks++;
      }
    }
  }
  return stats;
}


//
// BDI related helpers.
//

void Compressor::insert(pointer_t address, size_t size, data_t data) {
  // get basic information
  pointer_t index = getSet(address);
  pointer_t bib = getBib(address);
  pointer_t tag_value = getTag(address);

  // get tags and data
  vector<byte_t>* data_array = &data_store[index];
  list<Tag>* tags = &tag_store[index];
  Tag* tag = contains(tags, tag_value);

  // if this block is in the cache, decompress, then write.
  vector<byte_t> uncompressed(block_size);
  vector<byte_t> compressed;
  compression_t mode;
  if (tag) {
    decompress(*tag, *data_array, &uncompressed);
    deallocateTag(tag); // may need to find new home
  }
  writeBytes(data, bib, size, &uncompressed);
  compress(uncompressed, &compressed, &mode);

  // Are there any invalid tags? If not we need to evict.
  tag = allocateTag(tags);
  if (!tag) {
    evict(tags);
    tag = allocateTag(tags);
  }

  // allocate continuous segments for compressed cache line
  size_t new_size = align(compressed.size(), SEGMENT_SIZE);
  int start_seg;
  while ((start_seg = space(*tags, new_size)) < 0) {
    evict(tags);
  }
  int start = start_seg * SEGMENT_SIZE;

  // finally copy the data in and init the tag
  tag->Allocate(getTag(address), mode, start_seg);
  copy(compressed, 0, data_array, start, tag->size);

  // update the usage stats
  tag->age = 0;
  touchTag(tags, *tag);
  used += tag->size_aligned;
}

// if every block is invalid, this does nothing
void Compressor::evict(list<Tag>* tags) {
  Tag* oldest = NULL;
  list<Tag>::reverse_iterator iter;
  for (iter = tags->rbegin(); iter != tags->rend(); ++iter) {
    if (iter->valid)
      oldest = &(*iter);
  }
  if (oldest)
    deallocateTag(oldest);
  evictions++;
}

int Compressor::space(const list<Tag>& tags, size_t size) {
  int num_segs = block_size / SEGMENT_SIZE;
  int start = -1;
  size_t space = 0;
  for (int seg = 0; seg < num_segs; seg++) {
    if (!inUse(tags, seg)) {
      if (space == 0)
        start = seg;
      space += SEGMENT_SIZE;
    } else {
      space = 0;
    }
    // if we have enough space, return the start index
    if (space >= size)
      return start;
  }
  return -1;
}

bool Compressor::inUse(const list<Tag>& tags, int segment) {
  list<Tag>::const_iterator iter;
  for (iter = tags.begin(); iter != tags.end(); ++iter) {
    const Tag& tag = *iter;
    if (tag.valid &&
        tag.seg_start <= segment && segment < tag.seg_start + tag.size_seg)
      return true;
  }
  return false;
}

// Take in a byte vector and try to compress it.
// The out_compression variable contains the compressed cache line.
void Compressor::compress(const vector<byte_t>& data,
    vector<byte_t>* out_data, compression_t* out_compression) {
  // try each scheme in order of best to worst compression
  if (allZeros(data)) {
    out_data->resize(1);
    out_data->at(0) = 0x00;
    *out_compression = ZEROS;
    return;
  }

  // all the same value
  segment_t rep_value;
  if (allSame(data, &rep_value)) {
    out_data->resize(SEGMENT_SIZE);
    writeBytes(rep_value, 0, SEGMENT_SIZE, out_data);
    *out_compression = REP_VALUES;
    return;
  }

  // general cases for base + delta compression
  if (packBaseDelta(data, BASE8, DELTA1, out_data)) {
    *out_compression = BASE8_DELTA1;
  } else if (packBaseDelta(data, BASE4, DELTA1, out_data)) {
    *out_compression = BASE4_DELTA1;
  } else if (packBaseDelta(data, BASE8, DELTA2, out_data)) {
    *out_compression = BASE8_DELTA2;
  } else if (packBaseDelta(data, BASE2, DELTA1, out_data)) {
    *out_compression = BASE2_DELTA1;
  } else if (packBaseDelta(data, BASE4, DELTA2, out_data)) {
    *out_compression = BASE4_DELTA2;
  } else if (packBaseDelta(data, BASE8, DELTA4, out_data)) {
    *out_compression = BASE8_DELTA4;
  }

  // nothing worked!
  else {
    out_data->resize(data.size());
    for (int i = 0; i < out_data->size(); i++)
      out_data->at(i) = data[i];
    *out_compression = NO_COMPRESS;
  }
}

void Compressor::decompress(const Tag& tag,
    const vector<byte_t>& data, vector<byte_t>* out_data) {
  // this should not happen.
  if (!tag.valid) {
    std::cerr << "[cmp] Decompress on invalid tag.";
    return;
  }

  // copy the compressed data from data_store
  vector<byte_t> compressed(tag.size);
  copy(data, tag.seg_start * SEGMENT_SIZE, &compressed, 0, tag.size);

  // create decompressed line
  segment_t base;
  out_data->resize(block_size);
  switch (tag.mode) {
    case ZEROS:
      for (int i = 0; i < out_data->size(); i++)
        out_data->at(i) = 0x00;
      break;
    case REP_VALUES:
      base = readBytes(compressed, 0, BASE8);
      for (int i = 0; i < out_data->size() / BASE8; i++)
        writeBytes(base, i * BASE8, BASE8, out_data);
      break;
    case BASE8_DELTA1: unpackBaseDelta(compressed, BASE8, DELTA1, out_data);
      break;
    case BASE8_DELTA2: unpackBaseDelta(compressed, BASE8, DELTA2, out_data);
      break;
    case BASE8_DELTA4: unpackBaseDelta(compressed, BASE8, DELTA4, out_data);
      break;
    case BASE4_DELTA1: unpackBaseDelta(compressed, BASE4, DELTA1, out_data);
      break;
    case BASE4_DELTA2: unpackBaseDelta(compressed, BASE4, DELTA2, out_data);
      break;
    case BASE2_DELTA1: unpackBaseDelta(compressed, BASE2, DELTA1, out_data);
      break;
    case NO_COMPRESS:
    default:
      copy(compressed, 0, out_data, 0, compressed.size());
  }
}


//
// General case Base + Delta compression logic.
//

// Note: out_line is only written to if compression was successful.
bool Compressor::packBaseDelta(const vector<byte_t>& line,
    size_t base_size, size_t delta_size, vector<byte_t>* out_line) {
  // determine the bounds for the deltas
  size_t shift = ((sizeof(delta_t) - delta_size)* BITS_IN_BYTE);
  delta_t delta_max = LLONG_MAX >> shift;
  delta_t delta_min = LLONG_MIN >> shift;

  // base is the first value
  segment_t base = readBytes(line, 0, base_size);

  // make sure all deltas are in correct range
  vector<delta_t> deltas;
  for (int offset = 0; offset < line.size(); offset += base_size) {
    segment_t value = readBytes(line, offset, base_size);
    delta_t delta = (delta_t)(value - base); // this should be pretty safe
    if (delta < delta_min || delta > delta_max)
      return false;
    deltas.push_back(delta);
  }

  // write the compressed cache line
  out_line->resize(base_size + (deltas.size() * delta_size));
  writeBytes(base, 0, base_size, out_line);
  for (int i = 0; i < deltas.size(); i++)
    writeBytes(deltas[i], base_size + (i * delta_size), delta_size, out_line);
  return true;
}

void Compressor::unpackBaseDelta(const vector<byte_t> compressed,
    size_t base_size, size_t delta_size, vector<byte_t>* out_data) {
  // grab base, decompress
  segment_t base = readBytes(compressed, 0, base_size);
  int num_deltas = (compressed.size() - base_size) / delta_size;

  // extract sign extended deltas
  for (int i = 0; i < num_deltas; i++) {

    // bit math to sign extend
    int offset = base_size + (i * delta_size);
    int shift = ((sizeof(segment_t) - delta_size) * BITS_IN_BYTE);
    segment_t unsigned_delta = readBytes(compressed, offset, delta_size);
    delta_t delta = ((delta_t)(unsigned_delta << shift)) >> shift;

    // store into decompressed line
    segment_t value = base + delta;
    writeBytes(value, i * base_size, base_size, out_data);
  }
}


//
// Special case compression
//

bool Compressor::allZeros(const vector<byte_t>& line) {
  for (int i = 0; i < line.size(); i++) {
    if (line[i]) return false;
  }
  return true;
}

bool Compressor::allSame(const vector<byte_t>& line, segment_t* out_value) {
  int size = sizeof(segment_t); // not SEGMENT_SIZE (avoid nasty type resize error)
  if (line.size() <= size) return true;
  // common case
  segment_t value = readBytes(line, 0, size);
  for (int i = 0; i < line.size(); i += size) {
    segment_t test = readBytes(line, i, size);
    if (test != value) return false;
  }
  return true;
}

//
// Tag related helpers.
//

Tag* Compressor::contains(list<Tag>* tags, pointer_t needle) {
  list<Tag>::iterator iter;
  for (iter = tags->begin(); iter != tags->end(); ++iter) {
    Tag* tag = &(*iter);
    if (tag->valid && tag->tag == needle)
      return tag;
  }
  return NULL;
}

Tag* Compressor::allocateTag(list<Tag>* tags) {
  list<Tag>::iterator iter;
  for (iter = tags->begin(); iter != tags->end(); ++iter) {
    Tag *tag = &(*iter);
    if (!tag->valid)
      return tag;
  }
  return NULL;
}

void Compressor::deallocateTag(Tag* tag) {
  if (!tag) return;
  used -= tag->size_aligned;
  tag->valid = false;
  tag->age = 0x0;
}

Tag* Compressor::touchTag(list<Tag>* tags, const Tag& tag) {
  tags->remove(tag);
  tags->push_front(tag);
  return &tags->front();
}


//
// Dimension related helpers.
//

pointer_t Compressor::getTag(pointer_t address) {
  return ((address >> set_bits) >> bib_bits) & mask(tag_bits);
}

pointer_t Compressor::getSet(pointer_t address) {
  return (address >> bib_bits) & mask(set_bits);
}

pointer_t Compressor::getBib(pointer_t address) {
  return address & mask(bib_bits);
}


//
// Debugging functions
//
void Compressor::Print() const {
  for (int i = 0; i < sets; i++) {
    const list<Tag>& tags = tag_store[i];
    cout << "set[" << i << "] tags: ";
    std::list<Tag>::const_iterator iter;
    for (iter = tags.begin(); iter != tags.end(); ++iter) {
      const Tag& tag = *iter;
      tag.Print();
      cout << " ";
    }
    cout << endl;
  }
}
