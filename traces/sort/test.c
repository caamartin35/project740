#include "test.h"

// set up tracer
trace_t trace;

void sort(node_t** a, int array_size) {
  int i;
  for (i = 0; i < array_size - 1; ++i) {
    int min = i;
    node_t* temp;
    for (int j = i + 1; j < array_size; ++j) {
      if (a[j]->data < a[min]->data) {
        min = j;
      }

      trace_load(&trace, &a[j]->data, sizeof(a[j]->data), a[j]->data);
      trace_load(&trace, &a[j]->flag, sizeof(a[j]->flag), a[j]->flag);
      trace_load(&trace, &a[j]->next, sizeof(a[j]->next), (data_t)a[j]->next);

      trace_load(&trace, &a[min]->data, sizeof(a[min]->data), a[min]->data);
      trace_load(&trace, &a[min]->flag, sizeof(a[min]->flag), a[min]->flag);
      trace_load(&trace, &a[min]->next, sizeof(a[min]->next), (data_t)a[min]->next);
    }

    temp = a[i];
    a[i] = a[min];
    a[min] = temp;

    trace_store(&trace, &a[min]->data, sizeof(a[i]->data), a[i]->data);
    trace_store(&trace, &a[min]->flag, sizeof(a[i]->flag), a[i]->flag);
    trace_store(&trace, &a[min]->next, sizeof(a[i]->next), (data_t)a[i]->next);

    trace_store(&trace, &a[i]->data, sizeof(a[min]->data), a[min]->data);
    trace_store(&trace, &a[i]->flag, sizeof(a[min]->flag), a[min]->flag);
    trace_store(&trace, &a[i]->next, sizeof(a[min]->next), (data_t)a[min]->next);
  }
}

int main(int argc, char const *argv[]) {
  // set up tracer
  trace_init(&trace, "../outputs/sort.trace");
  printf(">> Generating memory trace ...\n");

  // set up random numbers
  srand(7);

  // create a bunch of nodes
  int num_nodes = 1000;
  node_t** nodes = (node_t**) malloc(num_nodes * sizeof(node_t*));
  node_t* node_structs = (node_t*) malloc(num_nodes * sizeof(node_t));
  for (int i = 0; i < num_nodes; i++) {
    node_t* node = node_structs + i;
    node->data = rand();
    node->flag = rand() % 2;
    node->next = NULL;
    // save pointer
    nodes[i] = node;
    trace_store(&trace, &nodes[i], sizeof(node), (data_t)node);
    // use trace lib to record stores
    trace_store(&trace, &node->data, sizeof(node->data), node->data);
    trace_store(&trace, &node->flag, sizeof(node->flag), node->flag);
    trace_store(&trace, &node->next, sizeof(node->next), (data_t)node->next);
  }

  // sort the elements
  sort(nodes, num_nodes);

  // clean up
  printf(">> Done!\n");
  trace_destroy(&trace);

  return 0;
}
