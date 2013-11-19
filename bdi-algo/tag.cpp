#include "tag.h"

using std::cout;

Tag::Tag() {
  valid = false;
  seg_start = -1;
  seg_end = -1;
  size = -1;
}

Tag::Tag(int start, int end) {
  valid = false;
  SetInterval(start, end);
}

void Tag::SetInterval(int start, int end) {
  seg_start = start;
  seg_end = end;
  if (start == SEGMENT_NONE || end == SEGMENT_NONE) {
    size = SIZE_INVALID;
  } else {
    size = (end - start) * SEGMENT_SIZE;
  }
}

void Tag::Print() const {
  cout << "[v: " << valid << ", size: " << size << ", tag: " << tag << "]";
}
