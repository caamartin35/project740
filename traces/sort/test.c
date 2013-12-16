#include "test.h"

// set up tracer
trace_t trace;

void sort(node_t* nodes, int array_size) {
  int i;
  for (i = 0; i < array_size - 1; ++i) {
    int min = i;
    node_t temp;
    for (int j = i + 1; j < array_size; ++j) {
      if (nodes[j].data < nodes[min].data) {
        min = j;
      }

      // for the data comparison
      trace_load(&trace, &nodes[j].data, sizeof(nodes[j].data), nodes[j].data);
      trace_load(&trace, &nodes[min].data, sizeof(nodes[min].data), nodes[min].data);
    }

    temp = nodes[i];
    nodes[i] = nodes[min];
    nodes[min] = temp;

    trace_load(&trace, &temp.data, sizeof(nodes[i].data), nodes[i].data);
    trace_load(&trace, &temp.flag, sizeof(nodes[i].flag), nodes[i].flag);
    trace_load(&trace, &temp.next, sizeof(nodes[i].next), (data_t)nodes[i].next);

    trace_store(&trace, &nodes[min].data, sizeof(nodes[i].data), nodes[i].data);
    trace_store(&trace, &nodes[min].flag, sizeof(nodes[i].flag), nodes[i].flag);
    trace_store(&trace, &nodes[min].next, sizeof(nodes[i].next), (data_t)nodes[i].next);

    trace_store(&trace, &nodes[i].data, sizeof(nodes[min].data), nodes[min].data);
    trace_store(&trace, &nodes[i].flag, sizeof(nodes[min].flag), nodes[min].flag);
    trace_store(&trace, &nodes[i].next, sizeof(nodes[min].next), (data_t)nodes[min].next);
  }
}

int main(int argc, char const *argv[]) {
  // set up tracer
  trace_init(&trace, "../outputs/test.sort.trace");
  printf(">> Generating memory trace ...\n");

  // set up random numbers
  srand(7);

  // create a bunch of nodes
  int num_nodes = 5000;
  node_t* nodes = (node_t*) malloc(num_nodes * sizeof(node_t));
  for (int i = 0; i < num_nodes; i++) {
    node_t* node = nodes + i;
    node->data = rand();
    node->flag = rand() % 2;
    node->next = nodes + (rand() % num_nodes);
    // use trace lib to record stores
    trace_store(&trace, &node->data, sizeof(node->data), node->data);
    trace_store(&trace, &node->flag, sizeof(node->flag), node->flag);
    trace_store(&trace, &node->next, sizeof(node->next), (data_t)node->next);
  }

  // sort the elements
  sort(nodes, num_nodes);

  // clean up
  size_t working = 0;
  working += num_nodes * sizeof(node_t);
  printf(">> working set = %lu\n", working);
  printf(">> Done!\n");

  trace_destroy(&trace);
  return 0;
}
