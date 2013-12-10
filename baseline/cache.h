#ifndef __CACHE_H__
#define __CACHE_H__

#include <climits>
#include <iostream>
#include <iomanip>
#include <list>
#include <vector>

#include "../lib/math.h"
#include "../lib/types.h"

#include "tag.h"

class Cache {
 public:
  Cache(int size, int ways, int block_size);
  bool Load(pointer_t address, size_t size, data_t data);
  bool Store(pointer_t address, size_t size, data_t data);
  void Cycle();
  // data
  int size;
  int sets;
  int ways;
  int block_size;
  // stats
  int used;
  int requests;
  int hits;
  int misses;
  int evictions;

 private:
  // helpers
  void insert(pointer_t address, size_t size, data_t data);
  void evict(std::list<Tag>* tags);
  Tag* contains(std::list<Tag>* tags, pointer_t needle);
  Tag* allocateTag(std::list<Tag>* tags);
  void deallocateTag(Tag* tag);
  Tag* touchTag(std::list<Tag>* tags, const Tag& tag);
  // dimensions
  pointer_t getTag(pointer_t address);
  pointer_t getSet(pointer_t address);
  pointer_t getBib(pointer_t address);
  // save some bit-math
  int tag_bits;
  int set_bits;
  int bib_bits;
  // memory
  std::vector<std::list<Tag> > tagStore;
};

#endif
