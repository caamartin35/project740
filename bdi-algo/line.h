#ifndef __LINE_H__
#define __LINE_H__

#include "types.h"

class Line {
 public:
  Line();
  bool valid;
  pointer_t tag;
  data_t block;
};

#endif