#include "tag.h"

using std::cout;
using std::endl;
using std::vector;

Tag::Tag() {
  valid = false;
}


void Tag::Allocate(pointer_t tag, const bytes_t& data) {
  valid = true;
  this->tag = tag;
  this->data = data;
  mode = BaseDelta::Compress(this->data);
  size = BaseDelta::Size(this->mode);
  size_aligned = ((size + (SEGMENT_SIZE - 1)) / SEGMENT_SIZE) * SEGMENT_SIZE;
}


void Tag::Deallocate() {
  valid = false;
}


//
// Utility functions
//

void Tag::Print() const {
  cout << "[v: " << valid << ", size: " << size << ", tag: " << tag << "]";
}

bool Tag::operator== (const Tag& rhs) {
  bool equals = valid && rhs.valid && (tag == rhs.tag);
  return equals;
}
