#ifndef __NODE_H__
#define __NODE_H__

#include <stdlib.h>

#define NIL ((HANDLE *) 0x0)

struct node {
  int value;
  struct node *left;
  struct node *right;
};

typedef struct node HANDLE;

typedef struct future_cell_int{
  HANDLE *value;
} future_cell_int;

// extern void *malloc(unsigned);

#endif
