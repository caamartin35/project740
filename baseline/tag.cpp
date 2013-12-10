#include "tag.h"

using std::cout;
using std::endl;

Tag::Tag() {
  valid = false;
}


void Tag::Allocate(pointer_t tag) {
  valid = true;
  this->tag = tag;
}

void Tag::Deallocate() {
  valid = false;
}

void Tag::Print() const {
  cout << "Tag<v: " << valid << ", tag: " << tag << ">";
}

bool Tag::operator== (const Tag& rhs) {
  bool equals = valid && rhs.valid && (tag == rhs.tag);
  return equals;
}
