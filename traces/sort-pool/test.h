#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <stdlib.h>

#include "../trace.h"

struct node_s;

typedef struct node_s {
  int data;
  char flag;
  struct node_s* next;
} node_t;

#endif
