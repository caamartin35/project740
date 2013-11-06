#include "test.h"

int main(int argc, char const *argv[]) {
  // set up tracer
  trace_t trace;
  trace_init(&trace, "test.trace");

  // set up random numbers
  srand(7);

  // create a bunch of nodes
  int num_nodes = 500;
  node_t* nodes = (node_t*) malloc(num_nodes * sizeof(node_t));
  for (int i = 0; i < num_nodes; i++) {
    node_t* node = nodes + i;
    node->data = rand();
    node->flag = rand() % 2;
    node->next = NULL;
    // use trace lib to record stores
    trace_store(&trace, &node->data, sizeof(node->data), node->data);
    trace_store(&trace, &node->flag, sizeof(node->flag), node->flag);
  }

  printf("Hello World!\n");

  return 0;
}
