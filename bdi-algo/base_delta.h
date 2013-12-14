#ifndef __BASE_DELTA_H__
#define __BASE_DELTA_H__

#include <iostream>

#include "../lib/types.h"

#include "util.h"

#define BASE2   2
#define BASE4   4
#define BASE8   8
#define DELTA1  1
#define DELTA2  2
#define DELTA4  4

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

class BaseDelta {
 public:
  static compression_t Compress(const bytes_t& data);
  static int Size(compression_t mode);

 private:
  static bool allZeros(const bytes_t& line);
  static bool allSame(const bytes_t& line, segment_t* out_value);
  static bool pack(const bytes_t& line, size_t base_size, size_t delta_size);
};

#endif