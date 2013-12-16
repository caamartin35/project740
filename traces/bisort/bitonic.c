/* For copyright information, see olden_v1.0/COPYRIGHT */

/* =================== PROGRAM bitonic===================== */
/* UP - 0, DOWN - 1 */
#include <stdio.h>

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
int flag=0,foo=0;


//
// Macros
//
#define LocalNewNode(h,v) { \
  h = (HANDLE *) malloc(sizeof(struct node)); \
  h->value = v; \
  h->left = NIL; \
  h->right = NIL; \
  trace_store(&trace, &h->value, sizeof(h->value), (data_t)h->value); \
  trace_store(&trace, &h->left, sizeof(h->left), (data_t)h->left); \
  trace_store(&trace, &h->right, sizeof(h->right), (data_t)h->right); \
};

#define NewNode(h,v,procid) LocalNewNode(h,v)


//
// Functions
//
void InOrder(HANDLE *h) {
  HANDLE *l, *r;
  if ((h != NIL)) {
    l = h->left;
    r = h->right;
    trace_load(&trace, &h->left, sizeof(h->left), (data_t)h->left);
    trace_load(&trace, &h->right, sizeof(h->right), (data_t)h->right);
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
    trace_store(&trace, &h->left, sizeof(h->left), (data_t)h->left);
    trace_store(&trace, &h->right, sizeof(h->right), (data_t)h->right);
  } else {
    h = 0;
  }
  return h;
}

void SwapValue(HANDLE *l, HANDLE *r) {
  int temp,temp2;
  temp = l->value;
  temp2 = r->value;
  trace_load(&trace, &l->value, sizeof(l->value), (data_t)l->value);
  trace_load(&trace, &r->value, sizeof(r->value), (data_t)r->value);
  r->value = temp;
  l->value = temp2;
  trace_store(&trace, &l->value, sizeof(l->value), (data_t)l->value);
  trace_store(&trace, &r->value, sizeof(r->value), (data_t)r->value);
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
  trace_store(&trace, &l->left, sizeof(l->left), (data_t)l->left);
  trace_store(&trace, &r->left, sizeof(r->left), (data_t)r->left);
  trace_store(&trace, &l->value, sizeof(l->value), (data_t)l->value);
  trace_store(&trace, &r->value, sizeof(r->value), (data_t)r->value);
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
  /*printf("Swap Val Right l 0x%x,r 0x%x val: %d %d\n",l,r,lval,rval);*/
  trace_store(&trace, &l->right, sizeof(l->right), (data_t)l->right);
  trace_store(&trace, &r->right, sizeof(r->right), (data_t)r->right);
  trace_store(&trace, &l->value, sizeof(l->value), (data_t)l->value);
  trace_store(&trace, &r->value, sizeof(r->value), (data_t)r->value);
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
  trace_load(&trace, &root->value, sizeof(root->value), (data_t)root->value);
  trace_load(&trace, &root->left, sizeof(root->left), (data_t)root->left);
  trace_load(&trace, &root->right, sizeof(root->right), (data_t)root->right);

  // exchange logic
  rightexchange = ((rv > spr_val) ^ dir);
  if (rightexchange) {
    root->value = spr_val;
    spr_val = rv;
    trace_store(&trace, &root->value, sizeof(root->value), (data_t)root->value);
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
      trace_load(&trace, &pl->value, sizeof(pl->value), (data_t)pl->value);
      trace_load(&trace, &pl->left, sizeof(pl->left), (data_t)pl->left);
      trace_load(&trace, &pl->right, sizeof(pl->right), (data_t)pl->right);
      trace_load(&trace, &pr->value, sizeof(pr->value), (data_t)pr->value);
      trace_load(&trace, &pr->left, sizeof(pr->left), (data_t)pr->left);
      trace_load(&trace, &pr->right, sizeof(pr->right), (data_t)pr->right);

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

  trace_load(&trace, &root->left, sizeof(root->left), (data_t)root->left);
  if (root->left != NIL) {
    int value;
    rl = root->left;
    rr = root->right;
    value = root->value;
    trace_load(&trace, &root->value, sizeof(root->value), (data_t)root->value);
    trace_load(&trace, &root->left, sizeof(root->left), (data_t)root->left);
    trace_load(&trace, &root->right, sizeof(root->right), (data_t)root->right);

    root->value = Bimerge(rl,value,dir);
    trace_store(&trace, &root->value, sizeof(root->value), (data_t)root->value);

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
  trace_load(&trace, &root->left, sizeof(root->left), (data_t)root->left);
  if (root->left == NIL) { /* <---- 8.7% load penalty */
    trace_load(&trace, &root->value, sizeof(root->value), (data_t)root->value);
    if ((root->value > spr_val) ^ dir) {
      val = spr_val;
      spr_val = root->value;
      trace_load(&trace, &root->value, sizeof(root->value), (data_t)root->value);
      root->value =val;
      trace_store(&trace, &root->value, sizeof(root->value), (data_t)root->value);
    }
  } else {
    int ndir;
    l = root->left;
    r = root->right;
    val = root->value;
    trace_load(&trace, &root->value, sizeof(root->value), (data_t)root->value);
    trace_load(&trace, &root->left, sizeof(root->left), (data_t)root->left);
    trace_load(&trace, &root->right, sizeof(root->right), (data_t)root->right);

    /*printf("root 0x%x, l 0x%x, r 0x%x\n", root,l,r);*/
    root->value=Bisort(l,val,dir);
    trace_store(&trace, &root->value, sizeof(root->value), (data_t)root->value);

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
  trace_init(&trace, "../outputs/test.bisort.trace");

  // parse arguments
  n = dealwithargs(argc,argv);

  printf("Bisort with %d size of dim %d\n", n, NDim);

  h = RandTree(n,12345768,0,0);
  sval = rand_num(245867) % RANGE;
  if (flag) {
    InOrder(h);
    printf("%d\n",sval);
  }
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

  // working set calculation
  size_t working = 0;
  working += n * sizeof(int);
  working += 2 * n * sizeof(struct node*);
  printf("working set = %luKB\n", working);

  trace_destroy(&trace);
  return 0;
}







