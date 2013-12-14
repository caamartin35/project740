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
  list<Tag>* tags = &tag_store[index];
  Tag* tag = contains(tags, tag_value);

  // if this block is in the cache, decompress, then write.
  bytes_t line(block_size);
  if (tag) {
    used -= tag->size_aligned;
    line = tag->data;
  }
  writeBytes(data, bib, size, &line);

  // make sure to get a tag!
  if (!tag) tag = allocateTag(tags);

  // allocate tag, see if we are over capacity
  tag->Allocate(tag_value, line);
  tag = touchTag(tags, *tag);
  while (space(*tags) > (ways * block_size))
    evict(tags);

  // update the usage stats
  used += tag->size_aligned;
}

// if every block is invalid, this does nothing
void Compressor::evict(list<Tag>* tags) {
  Tag* oldest = NULL;
  list<Tag>::reverse_iterator iter;
  for (iter = tags->rbegin(); iter != tags->rend(); ++iter) {
    if (iter->valid) {
      oldest = &(*iter);
      break;
    }
  }
  if (oldest) {
    deallocateTag(oldest);
    evictions++;
  }
}

int Compressor::space(const list<Tag>& tags) {
  int space = 0;
  list<Tag>::const_iterator iter;
  for (iter = tags.begin(); iter != tags.end(); ++iter) {
    const Tag& tag = *iter;
    if (tag.valid)
      space += tag.size_aligned;
  }
  return space;
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
  // evict then return one
  evict(tags);
  return allocateTag(tags);
}

void Compressor::deallocateTag(Tag* tag) {
  if (!tag) return;
  used -= tag->size_aligned;
  tag->Deallocate();
}

Tag* Compressor::touchTag(list<Tag>* tags, Tag tag) {
  tags->remove(tag);
  tags->push_front(tag);
  if (tags->size() != ways * 2) {
    cout << "tags.size = " << tags->size() << endl;
    exit(1);
  }
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
