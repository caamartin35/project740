#include "trace.h"

void trace_init(trace_t* trace, const char *fname) {
  trace->name = fname;
  trace->file = fopen(fname, "w");
  trace->loads = 0;
  trace->stores = 0;
}

void trace_load(trace_t* trace, void *address, size_t size) {
  trace->loads++;
  // write to the trace file
  fprintf(trace->file, "access %p %zu\n", address, size);
}

void trace_store(trace_t* trace, void *address,
    size_t size, unsigned long long data) {
  trace->loads++;
  // write to the trace file
  fprintf(trace->file, "access %p %zu %llu\n", address, size, data);
}
