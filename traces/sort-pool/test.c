#include "test.h"

// set up tracer
trace_t trace;

void sort(int* values, char* flags, node_t** nexts, int len) {
  int i;
  for (i = 0; i < len - 1; ++i) {
    int min = i;
    node_t temp;
    // find min and swap
    for (int j = i + 1; j < len; ++j) {
      if (values[j] < values[min]) {
        min = j;
      }
      // for the comparison
      trace_load(&trace, &values[j], sizeof(values[j]), values[j]);
      trace_load(&trace, &values[min], sizeof(values[min]), values[min]);
    }

    temp.data = values[i];
    temp.flag = flags[i];
    temp.next = nexts[i];
    values[i] = values[min];
    flags[i] = flags[min];
    nexts[i] = nexts[min];
    values[min] = temp.data;
    flags[min] = temp.flag;
    nexts[min] = temp.next;

    trace_load(&trace, &temp.data, sizeof(values[i]), values[i]);
    trace_load(&trace, &temp.flag, sizeof(flags[i]), flags[i]);
    trace_load(&trace, &temp.next, sizeof(nexts[i]), (data_t)nexts[i]);

    trace_store(&trace, &values[i], sizeof(values[min]), values[min]);
    trace_store(&trace, &flags[i], sizeof(flags[min]), flags[min]);
    trace_store(&trace, &nexts[i], sizeof(nexts[min]), (data_t)nexts[min]);

    trace_store(&trace, &values[min], sizeof(values[i]), values[i]);
    trace_store(&trace, &flags[min], sizeof(flags[i]), flags[i]);
    trace_store(&trace, &nexts[min], sizeof(nexts[i]), (data_t)nexts[i]);
  }
}

int main(int argc, char const *argv[]) {
  // set up tracer
  trace_init(&trace, "../outputs/test.sort.pool.trace");
  printf(">> Generating memory trace ...\n");

  // set up random numbers
  srand(7);

  // create a bunch of nodes
  int num_nodes = 5000;
  int* node_datas = (int*) malloc(num_nodes * sizeof(int));
  char* node_flags = (char*) malloc(num_nodes * sizeof(char));
  node_t** node_nexts = (node_t**) malloc(num_nodes * sizeof(node_t*));

  // initialize in mutliple loops
  for (int i = 0; i < num_nodes; i++) {
    node_datas[i] = rand();
    trace_store(&trace, &node_datas[i], sizeof(node_datas[i]), node_datas[i]);

    node_flags[i] = rand() % 2;
    trace_store(&trace, &node_flags[i], sizeof(node_flags[i]), node_flags[i]);

    node_nexts[i] = (node_t*)(node_datas + (rand() % num_nodes));
    trace_store(&trace, &node_nexts[i], sizeof(node_nexts[i]), (data_t)node_nexts[i]);
  }

  // sort the elements
  sort(node_datas, node_flags, node_nexts, num_nodes);

  // clean up
  size_t working = 0;
  working += num_nodes * sizeof(node_t);
  printf(">> working set = %lu\n", working);
  printf(">> Done!\n");

  trace_destroy(&trace);
  return 0;
}
