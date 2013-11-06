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
  tag_bits = (sizeof(pointer_t) << 3) - set_bits - bib_bits;
  cout << tag_bits << endl;

  // allocate cache memory
  tag_store.resize(sets);
  for (int i = 0; i < tag_store.size(); i++) {
    tag_store[i].resize(2 * ways);
  }
  data_store.resize(sets);
  for (int i = 0; i < data_store.size(); i++) {
    data_store[i].resize(block_size);
  }

  // initialize counters
  hits = 0;
  misses = 0;
  evictions = 0;
}

void Compressor::Load(pointer_t address, size_t size, data_t data) {
  // pointer_t index = get_set(address);
  // pointer_t tag = get_tag(address);
  // vector<Line>& set = cache[index];
  // Line* line = contains(set, tag);
}

void Compressor::Store(pointer_t address, size_t size, data_t data) {
  int index = get_set(address);
  vector<Tag>& tags = tag_store[index];
  pointer_t needle = get_tag(address);
  Tag *tag = contains(tags, needle);
  if (!tag) {
    misses++;
  } else {
    // check if we need to evict
    // then do this
    hits++;
    tag->valid = true;
    tag->tag = needle;
  }
}

//
// Debugging functions
//
void Compressor::Print() {
  for (int i = 0; i < sets; i++) {
    vector<Tag>& tags = tag_store[i];
    cout << "set " << i << " <tags> ";
    for (int j = 0; j < tags.size(); j++) {
      Tag& tag = tags[j];
      cout << tag.valid << " ";
    }
    cout << endl;
  }
}

//
// Private functions
//
Tag* Compressor::contains(vector<Tag>& tags, pointer_t needle) {
  for (int i = 0; i < tags.size(); i++) {
    Tag& tag = tags[i];
    if (tag.valid && tag.tag == needle)
      return &tag;
  }
  return NULL;
}

pointer_t Compressor::get_tag(pointer_t address) {
  return ((address >> set_bits) >> bib_bits) & mask(tag_bits);
}

pointer_t Compressor::get_set(pointer_t address) {
  return (address >> bib_bits) & mask(set_bits);
}

pointer_t Compressor::get_bib(pointer_t address) {
  return address & mask(bib_bits);
}

pointer_t Compressor::mask(int len) {
  pointer_t mask = 0x0;
  while (len--) {
    mask = (mask << 1) | 0x1;
  }
  return mask;
}

int Compressor::log2(int n) {
  int i = 0;
  while (n >>= 1) {
    i++;
  }
  return i;
}

