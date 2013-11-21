#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <climits>
#include <iostream>
#include <iomanip>
#include <vector>

#include "tag.h"
#include "types.h"
#include "util.h"

class Compressor {
 public:
  Compressor(int size, int ways, int block_size);
  void Load(pointer_t address, size_t size, data_t data);
  void Store(pointer_t address, size_t size, data_t data);
  void Cycle();
  void Print() const;
  // data
  int size;
  int sets;
  int ways;
  int block_size;
  // stats
  int hits;
  int misses;
  int evictions;

 private:
  // compression
  void decompress(const Tag& tag, const std::vector<byte_t>& data, std::vector<byte_t>* out_data);
  void compress(const std::vector<byte_t>& data, std::vector<byte_t>* out_data, compression_t* out_compression);
  bool allZeros(const std::vector<byte_t>& line);
  bool allSame(const std::vector<byte_t>& line, segment_t* out_value);
  bool packBaseDelta(const std::vector<byte_t>& line, size_t base_size, size_t delta_size, std::vector<byte_t>* out_line);
  void unpackBaseDelta(const std::vector<byte_t> compressed, size_t base, size_t delta, std::vector<byte_t>* out_data);
  // helpers
  void insert(pointer_t address, size_t size, data_t data);
  size_t evict(std::vector<Tag>* tags, std::vector<byte_t>* data);
  Tag* contains(std::vector<Tag>* tags, pointer_t needle);
  Tag* allocateTag(std::vector<Tag>* tags);
  // dimensions
  pointer_t getTag(pointer_t address);
  pointer_t getSet(pointer_t address);
  pointer_t getBib(pointer_t address);
  // save some bit-math
  int tag_bits;
  int set_bits;
  int bib_bits;
  // memory
  std::vector<std::vector<Tag> > tag_store;
  std::vector<std::vector<byte_t> > data_store;
};

#endif