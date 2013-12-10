#include "math.h"

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
