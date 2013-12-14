#ifndef __TAG_H__
#define __TAG_H__

#include <iostream>
#include <vector>

#include "../lib/types.h"

#include "base_delta.h"

#define SEGMENT_NONE      -1
#define SEGMENT_SIZE       8

class Tag {
 public:
  Tag();
  void Allocate(pointer_t tag, const bytes_t& data);
  void Deallocate();
  void Print() const;
  bool operator== (const Tag& rhs);

  // metadata
  bool valid;
  compression_t mode;
  pointer_t tag;

  // data
  int size;
  int size_aligned;
  bytes_t data;
};

#endif