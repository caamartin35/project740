#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <iostream>
#include <iomanip>
#include <vector>

#include "tag.h"
#include "types.h"

#define BITS_IN_BYTE 8

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
  void insert(pointer_t address, size_t size, data_t data);
  segment_t read_bytes(const std::vector<byte_t>& line, int offset, size_t length);
  void write_bytes(data_t data, int offset, size_t length, std::vector<byte_t>* line);
  Tag* contains(std::vector<Tag>& tags, pointer_t needle);
  // compression
  void decompress(const Tag& tag, std::vector<byte_t>* out_data);
  void compress(const std::vector<byte_t>& data, std::vector<byte_t>* out_data, compression_t* out_compression);
  bool allZeros(const std::vector<byte_t>& line);
  bool allSame(const std::vector<byte_t>& line, segment_t* out_value);
  // tag setup
  void setup_tags();
  // dimensions
  pointer_t get_tag(pointer_t address);
  pointer_t get_set(pointer_t address);
  pointer_t get_bib(pointer_t address);
  pointer_t mask(int len);
  int log2(int n);
  // save some bit-math
  int tag_bits;
  int set_bits;
  int bib_bits;
  // memory
  std::vector<std::vector<Tag> > tag_store;
  std::vector<std::vector<byte_t> > data_store;
};

#endif