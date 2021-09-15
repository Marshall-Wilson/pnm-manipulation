/*
 *  uarray2b.c 
 *  written by Eliza Encherman (eenche01) and Marshall Wilson (wwilso02)
 *  on 3/3/2021
 *  Purpose: TODO
 * 
 */


#include <uarray2b.h>
#include <stdio.h>
#include <stdlib.h>
#include <uarray2.h>
#include <uarray.h>
#include <assert.h>
#include <mem.h>
#include <math.h>
#include <stdbool.h>

#define T UArray2b_T

/* helper function declarations */
void add_a_block(int col, int row, UArray2_T uarray2, void *elemp, void *cl);
void free_a_block(int col, int row, UArray2_T uarray2, void *elemp, void *cl);
void map_a_block(int col, int row, UArray2_T uarray2, void *elemp, void *cl);
bool in_range(T array2b, int column, int row);


struct T {
    int width;    /* number of columns, at least 0 */
    int height;   /* number of rows, at least 0 */
    int size;    /* number of bytes in one element, > 0*/
    int blocksize;   /* number of elements per side of a block */
    UArray2_T block_arr; /* pointer 2D array of blocks */
};


/*  
 * new blocked 2d array
 * blocksize = square root of # of cells in block. 
 * blocksize < 1 is a checked runtime error
 */
extern T UArray2b_new (int width, int height, int size, int blocksize)
{
    assert(blocksize > 0 && size > 0);
    assert(width >= blocksize && height >= blocksize);
    
    /* make uarray2 that's width/blocksize x height/blocksize */
    T new_array;
    NEW(new_array);
    new_array->width = width;
    new_array->height = height;
    new_array->size = size;
    new_array->blocksize = blocksize;

    new_array->block_arr = UArray2_new(ceil((double) width / blocksize), 
                                       ceil((double) height / blocksize), 
                                       sizeof(UArray_T));

    UArray2_map_row_major(new_array->block_arr, add_a_block, &new_array);

    return new_array;
}


/* add_a_block
 *
 * given a UArray2 and the pointer to a cell of the UArray2 as parameters 3 & 4,
 * initializes a UArray_T and stores the pointer to that UArray_T in the cell of
 * the UArray2. 
 * 
 * Notes:
 *      apply function for UArray2
 *      parameters 1, 2, and 5 are not used.
 */
void add_a_block(int col, int row, UArray2_T uarray2, void *elemp, void *cl)
{
    T main_arr = *(T *)cl;
    *(UArray_T *)elemp = UArray_new(main_arr->blocksize * main_arr->blocksize, 
                                    main_arr->size);
                                    
    (void) col;
    (void) row;
    (void) uarray2;
}


/*
 *
 * 
 */
void free_a_block(int col, int row, UArray2_T uarray2, void *elemp, void *cl)
{
    /* helper function & all blocks were created, so elemp cannot be null */
    UArray_free((UArray_T *)elemp);

    (void) cl;
    (void) col;
    (void) row;
    (void) uarray2;
}


/* new blocked 2d array: blocksize as large as possible provided
 * block occupies at most 64KB (if possible)
 */
extern T UArray2b_new_64K_block(int width, int height, int size)
{
    return UArray2b_new(width, height, size, sqrt(64000/size));
}



/* frees UArray2b */
extern void  UArray2b_free (T *array2b)
{
    assert(array2b != NULL);
    UArray2_map_row_major((*array2b)->block_arr, free_a_block, NULL);
    UArray2_free(&(*array2b)->block_arr);
    FREE(*array2b);
}



/* UArray2b_width
 *  returns the width (number of cols) of the provided array
 *  Note: it is a CRE for array2b to be NULL
 */
extern int   UArray2b_width    (T  array2b)
{
    assert(array2b != NULL);
    return array2b->width;
}


/*UArray2b_height
 *  returns the height (number of rows) of the provided array
 *  Note: it is a CRE for array2b to be NULL
 */
extern int   UArray2b_height   (T  array2b) {
    assert(array2b != NULL);
    return array2b->height;
}


/*UArray2b_size
 *  returns the size (number of bytes) of each element in the provided array
 *  Note: it is a CRE for array2b to be NULL
 */
extern int   UArray2b_size     (T  array2b)
{
    assert(array2b != NULL);
    return array2b->size;
}


/*UArray2b_blocksize
 *  returns the blocksize (number of elements in any side of a block) in 
 *      the provided array
 *  Note: it is a CRE for array2b to be NULL
 */
extern int UArray2b_blocksize(T  array2b)
{
    assert(array2b != NULL);
    return array2b->blocksize;
}

/* return a pointer to the cell in the given column and row.
 * index out of range is a checked run-time error
 */
extern void *UArray2b_at(T array2b, int column, int row)
{
    assert(array2b != NULL);
    assert(in_range(array2b, column, row));
    int blkSize = array2b->blocksize;
    UArray_T *block = UArray2_at(array2b->block_arr, column / blkSize,
                                                             row / blkSize);
    return UArray_at(*block, blkSize * (row % blkSize) + (column % blkSize));
}

/* in_range
 * 
 * returns true if the given column and row are within the range of the array
 */
bool in_range(T array2b, int column, int row) 
{
    return (column >= 0 && row >= 0 && 
            column < array2b->width && row < array2b->height);
}


typedef void apply_fun(int col, int row, T array2b, void *elem, void *cl);


struct block_map_cl {
    void *cl;
    apply_fun *apply;
    UArray2b_T array2b;
};

/* visits every cell in one block before moving to another block */
extern void  UArray2b_map(T array2b, 
                          void apply(int col, int row, T array2b,
                                     void *elem, void *cl), 
                          void *cl)
{
    assert(array2b != NULL);
    assert(apply != NULL);

   struct block_map_cl bmc = {cl, apply, array2b};
   UArray2_map_row_major(array2b->block_arr, map_a_block, &bmc);

}


/*
 *
 * 
 * 
 */
void map_a_block(int col, int row, UArray2_T uarray2, void *elemp, void *cl)
{
    T array2b = ((struct block_map_cl *)cl)->array2b;
    int blocksize = UArray2b_blocksize(array2b);
    int col_modifier = col * blocksize;
    int row_modifier = row * blocksize;

    for(int i = 0; i < UArray_length(*(UArray_T *)elemp); i++) {
        int a2b_col = (i % blocksize) + col_modifier;
        int a2b_row = (i / blocksize) + row_modifier;

        if (in_range(array2b, a2b_col, a2b_row)) {
            ((struct block_map_cl *)cl)->apply(a2b_col, 
                                            a2b_row, 
                                            array2b,
                                            UArray_at(*(UArray_T *)elemp, i),
                                            ((struct block_map_cl *)cl)->cl); 
        }
        
    }

    (void) uarray2;
}

#undef T
