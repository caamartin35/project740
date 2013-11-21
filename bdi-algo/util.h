#ifndef __UTIL_H__
#define __UTIL_H__

#include <vector>

#include "types.h"

// read/writing data
void copy(const std::vector<byte_t>& src, int src_off,
    std::vector<byte_t>* dest, int dest_off, int length);
segment_t readBytes(const std::vector<byte_t>& line,
    int offset, size_t length);
void writeBytes(data_t data, int offset, size_t length,
    std::vector<byte_t>* out_line);

// math stuff
pointer_t mask(int len);
int log2(int n);

#endif