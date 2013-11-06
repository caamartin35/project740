#include "compressor.h"

using std::cout;
using std::endl;
using std::vector;

Compressor::Compressor(int size, int sets, int ways) {
  // configure the cache dimensions
  this->size = size;
  this->sets = sets;
  this->ways = ways;
  this->block_size = size / sets / ways;

  // compute the address masks
  set_bits = log2(sets);
  bib_bits = log2(block_size);
  tag_bits = log2(sizeof(pointer_t) << 3) - set_bits - bib_bits;

  // allocate cache memory
  cache.resize(sets);
  for (int i = 0; i < cache.size(); i++) {
    for (int j = 0; j < ways; j++) {
      Line line;
      cache[i].push_back(line);
    }
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
  // int index = get_set(address);
  // vector<Line>& set = cache[index];
}

//
// Debugging functions
//
void Compressor::Print() {
  for (int i = 0; i < sets; i++) {
    vector<Line>& set = cache[i];
    cout << "set " << i << " ";
    for (int j = 0; j < ways; j++) {
      Line& line = set[j];
      cout << line.valid << " ";
    }
    cout << endl;
  }
}

//
// Private functions
//
Line* Compressor::contains(vector<Line>& set, pointer_t tag) {
  for (int i = 0; i < set.size(); i++) {
    Line& line = set[i];
    if (line.tag == tag)
      return &line;
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

