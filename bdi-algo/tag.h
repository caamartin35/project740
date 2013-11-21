#ifndef __TAG_H__
#define __TAG_H__

#include <iostream>

#include "types.h"

#define SEGMENT_NONE -1
#define SEGMENT_SIZE 8
#define SIZE_INVALID -1
#define COMPRESSION_TYPES 9

typedef enum {
  ZEROS,
  REP_VALUES,
  BASE8_DELTA1,
  BASE8_DELTA2,
  BASE8_DELTA4,
  BASE4_DELTA1,
  BASE4_DELTA2,
  BASE2_DELTA1,
  NO_COMPRESS
} compression_t;

class Tag {
 public:
  Tag();
  void SetMode(compression_t mode);
  void Allocate(pointer_t tag, compression_t mode, int seg_start);
  void Print() const;
  // data
  bool valid;
  compression_t mode;
  pointer_t tag;
  // storage pointers
  int seg_start;
  int size;
  int size_seg;
  int size_aligned;
  // for LRU imitation
  unsigned int age;
};

#endif