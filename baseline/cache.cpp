#include "cache.h"

using std::cout;
using std::endl;
using std::list;
using std::vector;

Cache::Cache(int size, int ways, int block_size) {
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
  tagStore.resize(sets);
  for (int i = 0; i < tagStore.size(); i++) {
    for (int j = 0; j < ways; j++) {
      Tag tag;
      tagStore[i].push_back(tag);
    }
  }

  // memory utilization
  used = 0;

  // initialize counters
  requests = 0;
  hits = 0;
  misses = 0;
  evictions = 0;
}

void Cache::Cycle() {
  // any cycling logic (per mem operation)
  // maintain stats, etc.
}

bool Cache::Load(pointer_t address, size_t size, data_t data) {
  pointer_t index = getSet(address);
  pointer_t tag_value = getTag(address);
  list<Tag>* tags = &tagStore[index];
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

bool Cache::Store(pointer_t address, size_t size, data_t data) {
  int index = getSet(address);
  list<Tag>* tags = &tagStore[index];
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
    touchTag(tags, *tag);
    return true;
  }
}


//
// BDI related helpers.
//

void Cache::insert(pointer_t address, size_t size, data_t data) {
  // get basic information
  pointer_t index = getSet(address);
  pointer_t tagValue = getTag(address);

  // allocate a cache line
  list<Tag>* tags = &tagStore[index];
  Tag* tag = allocateTag(tags);

  // save
  tag->Allocate(tagValue);
  touchTag(tags, *tag);
  used += block_size;
}

// if every block is invalid, this does nothing
void Cache::evict(list<Tag>* tags) {
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


//
// Tag related helpers.
//

Tag* Cache::contains(list<Tag>* tags, pointer_t needle) {
  list<Tag>::iterator iter;
  for (iter = tags->begin(); iter != tags->end(); ++iter) {
    Tag* tag = &(*iter);
    if (tag->valid && tag->tag == needle)
      return tag;
  }
  return NULL;
}

Tag* Cache::allocateTag(list<Tag>* tags) {
  list<Tag>::iterator iter;
  for (iter = tags->begin(); iter != tags->end(); ++iter) {
    Tag *tag = &(*iter);
    if (!tag->valid)
      return tag;
  }

  // could not find a free tag
  evict(tags);
  return allocateTag(tags);
}

void Cache::deallocateTag(Tag* tag) {
  if (!tag) return;
  used -= block_size;
  tag->Deallocate();
}

Tag* Cache::touchTag(list<Tag>* tags, const Tag& tag) {
  tags->remove(tag);
  tags->push_front(tag);
  return &tags->front();
}


//
// Dimension related helpers.
//

pointer_t Cache::getTag(pointer_t address) {
  return ((address >> set_bits) >> bib_bits) & mask(tag_bits);
}

pointer_t Cache::getSet(pointer_t address) {
  return (address >> bib_bits) & mask(set_bits);
}

pointer_t Cache::getBib(pointer_t address) {
  return address & mask(bib_bits);
}
