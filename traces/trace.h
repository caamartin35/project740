#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdio.h>

typedef struct {
  const char *name;
  FILE* file;
  unsigned int loads;
  unsigned int stores;
} trace_t;

void trace_init(trace_t* trace, const char *fname);
void trace_load(trace_t* trace, void *address, size_t size);
void trace_store(trace_t* trace, void *address,
    size_t size, unsigned long long data);

#endif