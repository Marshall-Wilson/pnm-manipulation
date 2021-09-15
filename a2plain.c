#include <string.h>

#include <a2plain.h>
#include "uarray2.h"
 
/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/

typedef A2Methods_UArray2 A2;

static A2 new(int width, int height, int size)
{ 
  return UArray2_new(width, height, size);
}


static A2 new_with_blocksize(int width, int height, int size, 
                                            int blocksize)
{
  (void) blocksize;
  return UArray2_new(width, height, size);
}


static void a2free(A2 *array2p)
{
  UArray2_free((UArray2_T *)array2p);
}

static int width(A2 array2p)
{
  return UArray2_width(array2p);
}


static int height(A2 array2p)
{
  return UArray2_height(array2p);
}


static int size(A2 array2p)
{
  return UArray2_size(array2p);
}


static int blocksize(A2 array2p)
{
  (void) array2p;
  return 1; 
}

  
static A2Methods_Object *at(A2 array2p, int col, int row)
{
  return UArray2_at(array2p, col, row);
}

/* END OF OUR ADDED FUNCTIONS*/


static void map_row_major(A2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
  UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}


static void map_col_major(A2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
  UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
  A2Methods_smallapplyfun *apply; 
  void                    *cl;
};


static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
  struct small_closure *cl = vcl;
  (void)i;
  (void)j;
  (void)uarray2;
  cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
  struct small_closure mycl = { apply, cl };
  UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
  struct small_closure mycl = { apply, cl };
  UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
  new,
  new_with_blocksize,
  a2free,
  width,
  height,
  size,
  blocksize,
  at,
  map_row_major,      // map_row_major
  map_col_major,      // map_col_major
  NULL,               //map_block
  map_row_major,      // map_default
  small_map_row_major,// small_map_row_major
  small_map_col_major,// small_map_col_major
  NULL,               //small_map_block
  small_map_row_major// small_map_default
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
