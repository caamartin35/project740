#ifndef __TAG_H__
#define __TAG_H__

#include <iostream>

#include "../lib/types.h"

class Tag {
 public:
  Tag();
  void Allocate(pointer_t tag);
  void Deallocate();
  void Print() const;
  bool operator== (const Tag& rhs);
  // data
  bool valid;
  pointer_t tag;
};

#endif
