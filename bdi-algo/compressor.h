#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <iostream>
#include <vector>

#include "line.h"
#include "types.h"

class Compressor {
 public:
  Compressor(int size, int sets, int ways);
  void Load(pointer_t address, size_t size, data_t data);
  void Store(pointer_t address, size_t size, data_t data);
  void Print();
  int hits;
  int misses;
  int evictions;
 private:
  Line* contains(std::vector<Line>& set, pointer_t tag);
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
  std::vector<std::vector<Line> > cache;
};

#endif