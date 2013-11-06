#ifndef __TAG_H__
#define __TAG_H__

#include "types.h"

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
} Mode;

class Tag {
 public:
  Tag();
  bool valid;
  bool base0;
  Mode mode;
  pointer_t tag;

};

#endif