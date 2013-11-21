#include "util.h"

using std::cout;
using std::endl;
using std::vector;

//
// Reading writing and copying helpers
//

void copy(const vector<byte_t>& src, int src_off, vector<byte_t>* dest,
     int dest_off, int length) {
  // bounds checking
  if (src_off + length > src.size()) return;
  if (dest_off + length > dest->size()) return;
  // general case
  for (int i = 0; i < length; i++) {
    dest->at(dest_off + i) = src[src_off + i];
  }
}

// This function writes the bytes in the little endian format.
// The most we can read out from a line at a time is 8 bytes.
// @param {size_t} length The length to read out in bytes.
segment_t readBytes(const vector<byte_t>& line, int offset, size_t length) {
  segment_t data = 0x0;
  for (int i = offset; i < offset + length && i < line.size(); i++) {
    segment_t byte = line[i];
    data = (byte << (BITS_IN_BYTE * (i - offset))) + data;
  }
  return data;
}

void writeBytes(data_t data, int offset, size_t length, vector<byte_t>* out_line) {
  data_t masked_data = data & mask(length * BITS_IN_BYTE);
  for (int i = offset; i < offset + length && i < out_line->size(); i++) {
    byte_t byte = (byte_t) (masked_data & 0xFF);
    out_line->at(i) = byte;
    masked_data >>= BITS_IN_BYTE;
  }
}


//
// Math and bit related helpers
//

size_t align(size_t n, size_t base) {
  return ((n + base - 1) / base) * base;
}

pointer_t mask(int len) {
  pointer_t mask = 0x0;
  while (len--) {
    mask = (mask << 1) | 0x1;
  }
  return mask;
}

int log2(int n) {
  int i = 0;
  while (n >>= 1) {
    i++;
  }
  return i;
}
