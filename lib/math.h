#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

#include <cstdlib>

#include "types.h"

// math stuff
size_t align(size_t n, size_t base);
pointer_t mask(int len);
int log2(int n);

#endif
