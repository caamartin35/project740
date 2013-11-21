#include "compressor.h"

using std::cout;
using std::endl;
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
    tag_store[i].resize(2 * ways);  // twice as many tags
  }
  data_store.resize(sets);
  for (int i = 0; i < data_store.size(); i++) {
    data_store[i].resize(block_size * ways);
  }

  // initialize counters
  hits = 0;
  misses = 0;
  evictions = 0;
}

void Compressor::Load(pointer_t address, size_t size, data_t data) {
  pointer_t index = getSet(address);
  pointer_t tag_value = getTag(address);
  vector<Tag>* tags = &tag_store[index];
  Tag* tag = contains(tags, tag_value);
  if (!tag) {
    misses++;
    // insert()
  } else {
    hits++;
    tag->age = 0;
    // dont need to do anything, but w/e
    const vector<byte_t>& data = data_store[index];
    vector<byte_t> uncompressed(block_size);
    decompress(*tag, data, &uncompressed);

    // print?
    for (int i = 0; i < uncompressed.size(); i++)
      cout << std::setw(2) << std::setfill('0') << std::hex << (int)uncompressed[i] << " ";
    cout << std::dec << endl;
  }
}

void Compressor::Store(pointer_t address, size_t size, data_t data) {
  int index = getSet(address);
  vector<Tag>* tags = &tag_store[index];
  pointer_t needle = getTag(address);
  Tag *tag = contains(tags, needle);
  if (!tag) {
    misses++;
    insert(address, size, data);
  } else {
    hits++;
    insert(address, size, data);
  }
}

void Compressor::Cycle() {
  for (int i = 0; i < tag_store.size(); i++) {
    vector<Tag>* tags = &tag_store[i];
    for (int j = 0; j < tags->size(); j++) {
      tags->at(j).age++;
    }
  }
}


//
// BDI related helpers.
//
void Compressor::insert(pointer_t address, size_t size, data_t data) {
  int index = getSet(address);
  int bib = getBib(address);

  // set up the line for compression (we know its uncompressed size)
  // remember to decompress first!
  vector<byte_t> uncompressed(block_size);
  vector<byte_t> compressed;
  compression_t mode;
  writeBytes(data, bib, size, &uncompressed);
  compress(uncompressed, &compressed, &mode);

  // print some stuff out for now
  for (int i = 0; i < uncompressed.size(); i++)
    cout << std::setw(2) << std::setfill('0') << std::hex << (int)uncompressed[i] << " ";
  cout << std::dec << endl;
  for (int i = 0; i < compressed.size(); i++)
    cout << std::setw(2) << std::setfill('0') << std::hex << (int)compressed[i] << " ";
  cout << std::dec << endl;

  // Are there any invalid tags? If not we need to evict.
  vector<Tag>* tags = &tag_store[index];
  vector<byte_t>* data_array = &data_store[index];
  Tag* tag = allocateTag(tags);
  if (!tag) {
    //evict()
    // tag = allocateTag(tags);
  }

  // allocate continuous segments for compressed cache line
  // int new_size = compressed.size();
  int space = data_array->size();
  int start = data_array->size() - space;
  int start_seg = start / SEGMENT_SIZE;
  // for (int i = 0; i < tags->size(); i++) {
  //   if (tags->at(i).valid)
  //     space -= tags->at(i).size_aligned;
  // }
  // while (new_size > space) {
  //   space += evict()
  // }

  tag->Allocate(getTag(address), mode, start_seg);
  copy(compressed, 0, data_array, start, tag->size);
}


size_t Compressor::evict(vector<Tag>* tags, vector<byte_t>* data) {
  return 0;
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
  if (packBaseDelta(data, 8, 1, out_data)) {
    *out_compression = BASE8_DELTA1;
  } else if (packBaseDelta(data, 4, 1, out_data)) {
    *out_compression = BASE4_DELTA1;
  } else if (packBaseDelta(data, 8, 2, out_data)) {
    *out_compression = BASE8_DELTA2;
  } else if (packBaseDelta(data, 2, 1, out_data)) {
    *out_compression = BASE2_DELTA1;
  } else if (packBaseDelta(data, 4, 2, out_data)) {
    *out_compression = BASE4_DELTA2;
  } else if (packBaseDelta(data, 8, 4, out_data)) {
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

  // see the round trip stuff
  for (int i = 0; i < compressed.size(); i++)
    cout << std::setw(2) << std::setfill('0') << std::hex << (int)compressed[i] << " ";
  cout << std::dec << endl;

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

Tag* Compressor::contains(vector<Tag>* tags, pointer_t needle) {
  for (int i = 0; i < tags->size(); i++) {
    Tag* tag = &tags->at(i);
    if (tag->valid && tag->tag == needle)
      return tag;
  }
  return NULL;
}

Tag* Compressor::allocateTag(vector<Tag>* tags) {
  for (int i = 0; i < tags->size(); i++) {
    if (!(tags->at(i).valid)) return &tags->at(i);
  }
  return NULL;
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
    const vector<Tag>& tags = tag_store[i];
    cout << "set[" << i << "] tags: ";
    for (int j = 0; j < tags.size(); j++) {
      const Tag& tag = tags[j];
      tag.Print();
      cout << " ";
    }
    cout << endl;
  }
}
