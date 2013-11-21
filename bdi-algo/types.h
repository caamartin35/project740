#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstdint>

#define BITS_IN_BYTE 8

typedef uint8_t  byte_t;     // 1 byte
typedef uint64_t data_t;     // 8 bytes
typedef int64_t  delta_t;    // 8 bytes
typedef uint64_t segment_t;  // 8 bytes
typedef uint64_t pointer_t;  // 64 bits

#endif