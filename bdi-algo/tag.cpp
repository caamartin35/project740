#include "tag.h"

using std::cout;

Tag::Tag() {
  valid = false;
}


void Tag::Allocate(pointer_t tag, compression_t mode, int seg_start) {
  valid = true;
  this->tag = tag;
  SetMode(mode);
  this->seg_start = seg_start;
}


// It is a shame this is hard coded for 32 byte cache blocks.
// Maybe later we can read in a config file, or order the defines better.
void Tag::SetMode(compression_t mode) {
  this->mode = mode;
  switch (mode) {
    case ZEROS: size = 1; break;
    case REP_VALUES: size = 8; break;
    case BASE8_DELTA1: size = 12; break;
    case BASE8_DELTA2: size = 16; break;
    case BASE8_DELTA4: size = 24; break;
    case BASE4_DELTA1: size = 12; break;
    case BASE4_DELTA2: size = 20; break;
    case BASE2_DELTA1: size = 18; break;
    case NO_COMPRESS:
    default: size = 32;
  }
  // calulate the aligned size and start
  size_seg = (size + (SEGMENT_SIZE - 1)) / SEGMENT_SIZE;
  size_aligned = size_seg * SEGMENT_SIZE;
}

void Tag::Print() const {
  cout << "[v: " << valid << ", size: " << size << ", tag: " << tag << "]";
}
