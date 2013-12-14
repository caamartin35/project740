#include "base_delta.h"

using std::cout;
using std::endl;

compression_t BaseDelta::Compress(const bytes_t& data) {
  // try each scheme in order of best to worst compression
  if (allZeros(data)) return ZEROS;

  // all the same value
  segment_t rep_value;
  if (allSame(data, &rep_value)) return REP_VALUES;

  // general cases for base + delta compression
  // these are ordered by descending compression ratio.
  if (pack(data, BASE8, DELTA1)) {
    return BASE8_DELTA1;
  } else if (pack(data, BASE4, DELTA1)) {
    return BASE4_DELTA1;
  } else if (pack(data, BASE8, DELTA2)) {
    return BASE8_DELTA2;
  } else if (pack(data, BASE2, DELTA1)) {
    return BASE2_DELTA1;
  } else if (pack(data, BASE4, DELTA2)) {
    return BASE4_DELTA2;
  } else if (pack(data, BASE8, DELTA4)) {
    return BASE8_DELTA4;
  }

  // nothing worked!
  else {
    return NO_COMPRESS;
  }
}

// It is a shame this is hard coded for 32 byte cache blocks.
// Maybe later we can read in a config file, or order the defines better.
int BaseDelta::Size(compression_t mode) {
  int size = -1;
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
  return size;
}


//
// Special case compression
//

bool BaseDelta::allZeros(const bytes_t& line) {
  for (int i = 0; i < line.size(); i++) {
    if (line[i]) return false;
  }
  return true;
}

bool BaseDelta::allSame(const bytes_t& line, segment_t* out_value) {
  int size = sizeof(segment_t); // not SEGMENT_SIZE (avoid nasty type resize error)
  if (line.size() <= size) return true;
  // common case
  segment_t value = readBytes(line, 0, size);
  for (int i = 0; i < line.size(); i += size) {
    segment_t test = readBytes(line, i, size);
    if (test != value) return false;
  }
  return true;
}


//
// General case compression
//

bool BaseDelta::pack(const bytes_t& line, size_t base_size, size_t delta_size) {
  // determine the bounds for the deltas
  size_t shift = ((sizeof(delta_t) - delta_size) * BITS_IN_BYTE);
  delta_t delta_max = LLONG_MAX >> shift;
  delta_t delta_min = LLONG_MIN >> shift;

  // base is the first value
  segment_t base = readBytes(line, 0, base_size);

  // make sure all deltas are in correct range
  for (int offset = 0; offset < line.size(); offset += base_size) {
    segment_t value = readBytes(line, offset, base_size);
    delta_t delta = (delta_t)(value - base); // this should be pretty safe
    if (delta < delta_min || delta > delta_max)
      return false;
  }

  // we were able to compress the cache line
  return true;
}
