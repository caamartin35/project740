#include "trace.h"

void trace_init(trace_t* trace, const char *fname) {
  trace->name = fname;
  trace->file = fopen(fname, "w");
  trace->loads = 0;
  trace->stores = 0;
}

void trace_destroy(trace_t* trace) {
  trace->name = NULL;
  fclose(trace->file);
}

void trace_load(trace_t* trace, void *address, size_t size, data_t data) {
  trace->loads++;
  // write to the trace file
  fprintf(trace->file, "LD %p %zu\n", address, size);
}

void trace_store(trace_t* trace, void *address, size_t size, data_t data) {
  trace->loads++;
  // write to the trace file
  fprintf(trace->file, "ST %p %zu %llu\n", address, size, data);
}
