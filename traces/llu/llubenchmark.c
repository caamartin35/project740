/*
 * LLUBENCHMARK
 * Craig Zilles (zilles@cs.wisc.edu)
 * http://www.cs.wisc.edu/~zilles/llubenchmark.html
 *
 * This program is a linked list traversal micro-benchmark, which can
 * be used (among other things) to approximate the non-benchmark
 * Health.
 *
 * The benchmark executes for a proscribed number of iterations (-i),
 * and on every iteration the lists are traversed and potentially
 * extended.  The number of lists can be specified (-n) as well as the
 * size of the elements in the list (-s).  The initial length of the
 * lists can be set (-l) as well as the growth rate (-g).  The growth
 * rate must be non-negative, but can be a floating point number, in
 * which case random numbers are used to determine whether a list is
 * extended on a particular cycle (all lists are extended
 * independently).  If the -t option is specified, the insertion
 * occurs at the tail, otherwise at the head.  If the -d option is
 * specified, the elements are dirtied during the traversal (which
 * will necessitate a write-back when the data is evicted from the
 * cache).
 *
 * To approximate the non-benchmark Health, use the options:
 *     -i <num iterations> -g .333 -d -t -n 341
 *
 * (the growth rate of the lists in health is different for different
 * levels of the hierarchy and the constant .333 is just my
 * approximation of the growth rate).
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "../trace.h"

#if 0
#include <assert.h>
#else
#define assert(x)
#endif


// global trace file
trace_t trace;


/* This file should compile stand alone */

struct element {
  struct element *next;
  int count;
};

void usage(char *name) {
  printf("%s:\n", name);
  printf("-i <number of (I)terations>\n");
  printf("[-l <initial (L)ength of list, in elements>] (default 1)\n");
  printf("[-n <(N)umber of lists>] (default 1 list)\n");
  printf("[-s <(S)ize of element>] (default 32 bytes)\n");
  printf("[-g <(G)rowth rate per list, in elements per iteration>] (default 0)\n");
  printf("[-d] ((D)irty each element during traversal, default off)\n");
  printf("[-t] (insert at (T)ail of list, default off)\n");
}

#define ALLOC_SIZE 127 /* pick wierd num to break strides */
struct element *free_list = NULL;
int next_free = ALLOC_SIZE;
int element_size = 32;
int num_allocated = 0;

#if 0
struct element *
allocate() {
  if (next_free == ALLOC_SIZE) {
   next_free = 0;
   free_list = (struct element *) malloc (ALLOC_SIZE * element_size);
   assert(free_list != 0);
  }
  num_allocated ++;
  return (struct element *)
   (((char *)free_list) + ((next_free ++) * element_size));
}
#else
struct element * allocate() {
  num_allocated ++;
  return (struct element*)malloc(sizeof(struct element));
}
#endif

int main(int argc, char *argv[]) {
  // init the tracer
  trace_init(&trace, "../outputs/test.llu.trace");

  // configuration
  int max_iterations = 200, // 1000
   dirty = 1,
   num_lists = 98, // 196
   tail = 1,
   initial_length = 10;
  float growth_rate = 0.333;
  char c = 0;
  int i = 0, j = 0, k = 0;
  int accumulate = 0;
  trace_store(&trace, &accumulate, sizeof(accumulate), (data_t)accumulate);

  struct element **lists = NULL;
  float growth = 0.0;

  int arg = 1;

  printf("This benchmark modified to not use hard coded pool allocation!\n");
  while (arg < argc) {
   if ((argv[arg][0] != '-') || (argv[arg][2] != 0)) {
    printf("parse error in %s\n", argv[arg]);
    usage(argv[0]);
    return(-1);
   }
   c = argv[arg][1];
   arg ++;
   switch(c) {
   case 'd':    dirty = 1; break;
   case 'g':    growth_rate = atof(argv[arg++]);  break;
   case 'i':    max_iterations = atoi(argv[arg++]); break;
   case 'l':    initial_length = atoi(argv[arg++]); break;
   case 'n':    num_lists = atoi(argv[arg++]); break;
   case 's':    element_size = atoi(argv[arg++]); break;
   case 't':    tail = 1; break;
   default:
    printf("unrecognized option: %c\n", c);
    usage(argv[0]);
    return(-1);
   }
  }

  /* build lists */
  lists = (struct element **) malloc (num_lists * sizeof(struct element *));

  // initialize the lists
  for (i = 0; i < num_lists; i++) {
    lists[i] = NULL;
    trace_store(&trace, &lists[i], sizeof(lists[i]), (data_t)lists[i]);
  }


  for (i = 0; i < initial_length; i++) {
    for (j = 0; j < num_lists; j++) {
      struct element *e = allocate();
      e->next = lists[j];
      e->count = 0;
      lists[j] = e;
      trace_store(&trace, &e->next, sizeof(e->next), (data_t)e->next);
      trace_store(&trace, &e->count, sizeof(e->count), (data_t)e->count);
      trace_store(&trace, &lists[j], sizeof(lists[j]), (data_t)lists[j]);
    }
  }

  /* iterate */
  for (i = 0 ; i < max_iterations ; i ++) {
   if ((i % 1000) == 0) {
    printf("%d\n", i);
   }
   /* traverse lists */
   for (j = 0 ; j < num_lists ; j ++) {
    struct element *trav = lists[j];
    trace_load(&trace, &lists[j], sizeof(lists[j]), (data_t)lists[j]);
    while (trav != NULL) {
      trace_load(&trace, &accumulate, sizeof(accumulate), (data_t)accumulate);
      accumulate += trav->count;
      trace_store(&trace, &accumulate, sizeof(accumulate), (data_t)accumulate);
      if (dirty) {
       trace_load(&trace, &trav->count, sizeof(trav->count), (data_t)trav->count);
       trav->count++;
       trace_store(&trace, &trav->count, sizeof(trav->count), (data_t)trav->count);
      }
      trace_load(&trace, &trav->next, sizeof(trav->next), (data_t)trav->next);
      trav = trav->next;
    }
   }

   /* grow lists */
   growth += growth_rate;
   j = growth;
   growth -= j;
   for ( ; j > 0 ; j --) {
    for (k = 0 ; k < num_lists ; k++) {
      struct element *e = allocate();
      e->count = k+j;
      trace_store(&trace, &e->count, sizeof(e->count), (data_t)e->count);
      if (tail) {
       struct element *trav = lists[k];
       trace_load(&trace, &lists[k], sizeof(lists[k]), (data_t)lists[k]);
       while (trav->next != NULL) {
        trace_load(&trace, &trav->next, sizeof(trav->next), (data_t)trav->next);
        trav = trav->next;
       }

       // trace
       trav->next = e;
       trace_store(&trace, &trav->next, sizeof(trav->next), (data_t)trav->next);

       // trace
       e->next = NULL;
       trace_store(&trace, &e->next, sizeof(e->next), (data_t)e->next);
      } else {
       e->next = lists[k];
       trace_load(&trace, &lists[k], sizeof(lists[k]), (data_t)lists[k]);
       trace_store(&trace, &e->next, sizeof(e->next), (data_t)e->next);
       lists[k] = e;
       trace_store(&trace, &lists[k], sizeof(lists[k]), (data_t)lists[k]);
      }
    }
   }
  }

  // print the results
  printf("output = %d\n", accumulate);
  printf("num allocated %d\n", num_allocated);

  // figure out the working set size of the application
  size_t working_set = 0;
  working_set += (num_allocated * sizeof(struct element));
  working_set += (num_lists * sizeof(struct element*));
  printf("working set size = %lu", working_set / 1024);
  return 0;
}

