#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <iostream>
#include <vector>

#include "tag.h"
#include "types.h"

class Compressor {
 public:
  Compressor(int size, int ways, int block_size);
  void Load(pointer_t address, size_t size, data_t data);
  void Store(pointer_t address, size_t size, data_t data);
  void Print();
  int hits;
  int misses;
  int evictions;
 private:
  Tag* contains(std::vector<Tag>& tags, pointer_t needle);
  pointer_t get_tag(pointer_t address);
  pointer_t get_set(pointer_t address);
  pointer_t get_bib(pointer_t address);
  pointer_t mask(int len);
  int log2(int n);
  // data
  int size;
  int sets;
  int ways;
  int block_size;
  // save some bit-math
  int tag_bits;
  int set_bits;
  int bib_bits;
  // memory
  std::vector<std::vector<Tag> > tag_store;
  std::vector<std::vector<char> > data_store;
};

#endif