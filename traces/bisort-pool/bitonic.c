/* For copyright information, see olden_v1.0/COPYRIGHT */

/* =================== PROGRAM bitonic===================== */
/* UP - 0, DOWN - 1 */
#include <stdio.h>
#include <stdlib.h>

#include "../trace.h"

#include "node.h"   /* Node Definition */
#include "proc.h"   /* Procedure Types/Nums */

#define CONST_m1 10000
#define CONST_b 31415821
#define RANGE 100


//
// Function prototypes.
//
void InOrder(HANDLE *h);
int rand_num(int seed);


//
// Globals
//
trace_t trace;
int NumNodes, NDim;
int flag = 0;
int foo = 0;
int count = 0;
int* values;
node_t** lefts;
node_t** rights;


//
// Macros
//
#define LocalNewNode(h,v) { \
  h = (HANDLE *) malloc(sizeof(struct node)); \
  h->index = count; \
  h->value = v; \
  h->left = NIL; \
  h->right = NIL; \
  values[count] = v; \
  lefts[count] = NIL; \
  rights[count] = NIL; \
  trace_store(&trace, &values[count], sizeof(values[count]), (data_t)values[count]); \
  trace_store(&trace, &lefts[count], sizeof(lefts[count]), (data_t)lefts[count]); \
  trace_store(&trace, &rights[count], sizeof(rights[count]), (data_t)rights[count]); \
  count++; \
};

#define NewNode(h,v,procid) LocalNewNode(h,v)


//
// Functions
//
void InOrder(HANDLE *h) {
  HANDLE *l, *r;
  if (h != NIL) {
    l = h->left;
    r = h->right;
    // pooling sim
    int i = h->index;
    trace_load(&trace, &lefts[i], sizeof(lefts[i]), (data_t)lefts[i]);
    trace_load(&trace, &rights[i], sizeof(rights[i]), (data_t)rights[i]);
    InOrder(l);
    // static unsigned char counter = 0;
    // if (counter++ == 0)   /* reduce IO */
    //   printf("%d @ 0x%x\n",h->value, 0);
    InOrder(r);
  }
}

int mult(int p, int q) {
  int p1, p0, q1, q0;

  p1 = p/CONST_m1; p0 = p%CONST_m1;
  q1 = q/CONST_m1; q0 = q%CONST_m1;
  return ((p0*q1+p1*q0) % CONST_m1)*CONST_m1+p0*q0;
}

/* Generate the nth random # */
int skiprand(int seed, int n) {
  for (; n; n--) seed=rand_num(seed);
  return seed;
}

int rand_num(int seed) {
  return mult(seed,CONST_b)+1;
}

HANDLE* RandTree(int n, int seed, int node, int level) {
  int next_val,my_name;
  future_cell_int f_left, f_right;
  HANDLE *h;
  my_name=foo++;
  if (n > 1) {
    int newnode;
    if (level<NDim)
      newnode = node + (1 <<  (NDim-level-1));
    else
      newnode = node;
    seed = rand_num(seed);
    next_val=seed % RANGE;
    NewNode(h,next_val,node);
    f_left.value = RandTree((n/2),seed,newnode,level+1);
    f_right.value = RandTree((n/2),skiprand(seed,(n)+1),node,level+1);
    h->left = f_left.value;
    h->right = f_right.value;
    // pooling sim
    int i = h->index;
    lefts[i] = h->left;
    rights[i] = h->right;
    trace_store(&trace, &lefts[i], sizeof(lefts[i]), (data_t)lefts[i]);
    trace_store(&trace, &rights[i], sizeof(rights[i]), (data_t)rights[i]);
  } else {
    h = 0;
  }
  return h;
}

void SwapValue(HANDLE *l, HANDLE *r) {
  int temp,temp2;
  int i = l->index;
  int j = r->index;
  temp = l->value;
  temp2 = r->value;
  trace_load(&trace, &values[i], sizeof(values[i]), (data_t)values[i]);
  trace_load(&trace, &values[j], sizeof(values[j]), (data_t)values[j]);
  r->value = temp;
  l->value = temp2;
  values[j] = temp;
  values[i] = temp2;
  trace_store(&trace, &values[i], sizeof(values[i]), (data_t)values[i]);
  trace_store(&trace, &values[j], sizeof(values[j]), (data_t)values[j]);
}

void
/***********/
SwapValLeft(l,r,ll,rl,lval,rval)
/***********/
HANDLE *l;
HANDLE *r;
HANDLE *ll;
HANDLE *rl;
int lval, rval;
{
  r->value = lval;
  r->left = ll;
  l->left = rl;
  l->value = rval;
  int left = l->index;
  int right = r->index;
  values[left] = rval;
  values[right] = lval;
  lefts[right] = ll;
  lefts[left] = rl;
  trace_store(&trace, &values[left], sizeof(values[left]), (data_t)values[left]);
  trace_store(&trace, &values[right], sizeof(values[right]), (data_t)values[right]);
  trace_store(&trace, &lefts[left], sizeof(lefts[left]), (data_t)lefts[left]);
  trace_store(&trace, &lefts[right], sizeof(lefts[right]), (data_t)lefts[right]);
}


void
/************/
SwapValRight(l,r,lr,rr,lval,rval)
/************/
HANDLE *l;
HANDLE *r;
HANDLE *lr;
HANDLE *rr;
int lval, rval;
{
  r->value = lval;
  r->right = lr;
  l->right = rr;
  l->value = rval;
  // pooling
  int left = l->index;
  int right = r->index;
  values[left] = rval;
  values[right] = lval;
  rights[right] = lr;
  rights[left] = rr;
  /*printf("Swap Val Right l 0x%x,r 0x%x val: %d %d\n",l,r,lval,rval);*/
  trace_store(&trace, &values[left], sizeof(values[left]), (data_t)values[left]);
  trace_store(&trace, &values[right], sizeof(values[right]), (data_t)values[right]);
  trace_store(&trace, &rights[left], sizeof(rights[left]), (data_t)rights[left]);
  trace_store(&trace, &rights[right], sizeof(rights[right]), (data_t)rights[right]);
}

int
/********************/
Bimerge(root,spr_val,dir)
/********************/
HANDLE *root;
int spr_val,dir;

{ int rightexchange;
  int elementexchange;
  HANDLE *pl,*pll,*plr;
  HANDLE *pr,*prl,*prr;
  HANDLE *rl;
  HANDLE *rr;
  int rv,lv;

  /*printf("enter bimerge %x\n", root);*/
  rv = root->value;
  pl = root->left;
  pr = root->right;
  int index = root->index;
  trace_load(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
  trace_load(&trace, &lefts[index], sizeof(lefts[index]), (data_t)lefts[index]);
  trace_load(&trace, &rights[index], sizeof(rights[index]), (data_t)rights[index]);

  // exchange logic
  rightexchange = ((rv > spr_val) ^ dir);
  if (rightexchange) {
    root->value = spr_val;
    spr_val = rv;
    values[index] = spr_val;
    trace_store(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
  }

  while (pl != NIL)
    {
      /*printf("pl = 0x%x,pr = 0x%x\n",pl,pr);*/
      lv = pl->value;        /* <------- 8.2% load penalty */
      pll = pl->left;
      plr = pl->right;       /* <------- 1.35% load penalty */
      rv = pr->value;         /* <------ 57% load penalty */
      prl = pr->left;         /* <------ 7.6% load penalty */
      prr = pr->right;        /* <------ 7.7% load penalty */
      // pooling
      int left = pl->index;
      int right = pr->value;
      trace_load(&trace, &values[left], sizeof(values[left]), (data_t)values[left]);
      trace_load(&trace, &lefts[left], sizeof(lefts[left]), (data_t)lefts[left]);
      trace_load(&trace, &rights[left], sizeof(rights[left]), (data_t)rights[left]);
      trace_load(&trace, &values[right], sizeof(values[right]), (data_t)values[right]);
      trace_load(&trace, &lefts[right], sizeof(lefts[right]), (data_t)lefts[right]);
      trace_load(&trace, &rights[right], sizeof(rights[right]), (data_t)rights[right]);

      elementexchange = ((lv > rv) ^ dir);
      if (rightexchange)
        if (elementexchange)
          {
            SwapValRight(pl,pr,plr,prr,lv,rv);
            pl = pll;
            pr = prl;
          }
        else
          { pl = plr;
            pr = prr;
          }
      else
        if (elementexchange)
          {
            SwapValLeft(pl,pr,pll,prl,lv,rv);
            pl = plr;
            pr = prr;
          }
        else
          { pl = pll;
            pr = prl;
          }
    }

  trace_load(&trace, &lefts[index], sizeof(lefts[index]), (data_t)lefts[index]);
  if (root->left != NIL) {
    int value;
    rl = root->left;
    rr = root->right;
    value = root->value;
    trace_load(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
    trace_load(&trace, &lefts[index], sizeof(lefts[index]), (data_t)lefts[index]);
    trace_load(&trace, &rights[index], sizeof(rights[index]), (data_t)rights[index]);
    // recursion
    root->value = Bimerge(rl,value,dir);
    values[index] = root->value;
    trace_store(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);

    spr_val=Bimerge(rr,spr_val,dir);
  }

  /*printf("exit bimerge %x\n", root);*/
  return spr_val;
}

int
/*******************/
Bisort(root,spr_val,dir)
/*******************/
HANDLE *root;
int spr_val,dir;

{ HANDLE *l;
  HANDLE *r;
  int val;
  /*printf("bisort %x\n", root);*/
  int index = root->index;
  trace_load(&trace, &lefts[index], sizeof(lefts[index]), (data_t)lefts[index]);
  if (root->left == NIL) { /* <---- 8.7% load penalty */
    trace_load(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
    if ((root->value > spr_val) ^ dir) {
      val = spr_val;
      spr_val = root->value;
      trace_load(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
      root->value = val;
      values[index] = val;
      trace_store(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
    }
  } else {
    int ndir;
    l = root->left;
    r = root->right;
    val = root->value;
    trace_load(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);
    trace_load(&trace, &lefts[index], sizeof(lefts[index]), (data_t)lefts[index]);
    trace_load(&trace, &rights[index], sizeof(rights[index]), (data_t)rights[index]);

    /*printf("root 0x%x, l 0x%x, r 0x%x\n", root,l,r);*/
    root->value=Bisort(l,val,dir);
    values[index] = root->value;
    trace_store(&trace, &values[index], sizeof(values[index]), (data_t)values[index]);

    ndir = !dir;
    spr_val=Bisort(r,spr_val,ndir);
    spr_val=Bimerge(root,spr_val,dir);
  }
  /*printf("exit bisort %x\n", root);*/
  return spr_val;
}

int main(int argc, char **argv) {
  HANDLE *h;
  int sval;
  int n;

  // init the tracer
  trace_init(&trace, "../outputs/bisort.pool.trace");

  // parse arguments
  n = dealwithargs(argc,argv);
  printf("Bisort with %d size of dim %d\n", n, NDim);

  // pool allocate memory
  values = (int*) malloc(n * sizeof(int));
  lefts = (node_t**) malloc(n * sizeof(node_t*));
  rights = (node_t**) malloc(n * sizeof(node_t*));

  // create tree
  h = RandTree(n,12345768,0,0);
  sval = rand_num(245867) % RANGE;
  if (flag) {
    InOrder(h);
    printf("%d\n",sval);
  }

  // start
  printf("**************************************\n");
  printf("BEGINNING BITONIC SORT ALGORITHM HERE\n");
  printf("**************************************\n");

  sval=Bisort(h,sval,0);

  if (flag) {
    printf("Sorted Tree:\n");
    InOrder(h);
    printf("%d\n",sval);
  }

  sval=Bisort(h,sval,1);

  if (flag) {
    printf("Sorted Tree:\n");
    InOrder(h);
    printf("%d\n",sval);
  }

  return 0;
}







