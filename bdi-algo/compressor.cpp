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

