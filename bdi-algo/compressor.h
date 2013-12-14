#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <climits>
#include <iostream>
#include <iomanip>
#include <list>
#include <vector>

#include "../lib/stats.h"
#include "../lib/types.h"

#include "base_delta.h"
#include "tag.h"
#include "util.h"

class BaseDeltaStats : public Stats {
 public:
  int used_blocks;
  int zero_blocks;
  int rep_blocks;
  int medium_blocks;
  int large_blocks;
};

class Compressor {
 public:
  Compressor(int size, int ways, int block_size);
  bool Load(pointer_t address, size_t size, data_t data);
  bool Store(pointer_t address, size_t size, data_t data);
  void Cycle();
  BaseDeltaStats Stats() const;
  void Print() const;
  // data
  int size;
  int sets;
  int ways;
  int block_size;
  // stats
  int requests;
  int hits;
  int misses;
  int evictions;
  // more stats
  int used;

 private:
  // set operations
  void insert(pointer_t address, size_t size, data_t data);
  void evict(std::list<Tag>* tags);
  int space(const std::list<Tag>& tags);
  // tag operations
  Tag* contains(std::list<Tag>* tags, pointer_t needle);
  Tag* allocateTag(std::list<Tag>* tags);
  void deallocateTag(Tag* tag);
  Tag* touchTag(std::list<Tag>* tags, Tag tag);
  // dimensions
  pointer_t getTag(pointer_t address);
  pointer_t getSet(pointer_t address);
  pointer_t getBib(pointer_t address);
  // save some bit-math
  int tag_bits;
  int set_bits;
  int bib_bits;
  // memory
  std::vector<std::list<Tag> > tag_store;
};

#endif