/*
* uarray2.c
* Written By: Megan Gelement (mgelem01) and Marshall Wilson (wwilso02)
* Date:       February 19, 2021
* Summary:    uarray2.c is the implementation for a 2-dimensional version of
*               Hanson's UArray. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <uarray.h>
#include <assert.h>
#include <mem.h>
#include "uarray2.h"
#include "uarray2rep.h"

#define T UArray2_T

int get_array_index(T uarray2, int col, int row);

/*
* UArray2_new: allocates space for a new 2-D UArray with the number 
*              of columns specified by the width parameter, the number of 
*              rows specified by the height parameter, and the element size in 
*              bytes indicated by the size parameter. Returns a pointer
*              to the newly-allocated 2D array.
* Notes:       width, height, and size must be > 0. Raises Mem_failed if memory
*              allocation fails.
*/
T UArray2_new(int width, int height, int size)
{
    T uarray2;
    NEW(uarray2);
    UArray2Rep_init(uarray2, width, height, size);

    return uarray2;
}


/*
 * UArray2Rep_init: initializes the UArray2Rep_T struct with the provided 
 *                      data
 * Parameters:      T uarray2: the newly initialized UArray2_T
 *                  int width: the desired width of the 2D array
 *                  int height: the desired heigh of the 2D array
 *                  int size: the size of the elements to be stored in 
 *                      the 2D array
 * Returns:         None 
 * Notes:           T uarray2 cannot be null
 *                  Width and Height must be >= 0
 *                  Size must be > 0
 */
extern void UArray2Rep_init(T uarray2, int width, int height, int size)
{
    /* Error Checking */
    assert(uarray2 != NULL);
    assert(width >= 0 && height >= 0);
    assert(size > 0);

    /* Set descriptors */
    uarray2->width = width;
    uarray2->height = height;
    uarray2->count = width * height; 
    uarray2->size = size;
    uarray2->arr = UArray_new(uarray2->count, size);
}


/*
* UArray2_free: frees memory associated with UArray2 at the address of the 
*                   parameter.
* Notes:        raises CRE if uarray2 pointer is NULL
*/ 
void UArray2_free(T *uarray2)
{
    assert(uarray2 != NULL);
    assert(*uarray2 != NULL);
    UArray_free(&(*uarray2)->arr);
    FREE(*uarray2);
}

/*
 * UArray2_width: gets the number of columns in the uarray2 parameter.
 * Parameters:    T uarray2: the 2D uarray2 to query for width
 * Returns:       int width (number of columns) of the 2D uarray.
 */
int UArray2_width(T uarray2)
{
    return uarray2->width;
}


/*
 * UArray2_height: gets the number of rows in the uarray2 parameter.
 * Parameters:     T uarray2: the 2D uarray2 to query for height
 * Returns:        int height (number of rows) of the 2D uarray
 */
int UArray2_height(T uarray2)
{
    return uarray2->height;
}


/*
* UArray2_count: returns total number of elements in uarray2 parameter
* Notes:          analogous to UArray_length in 1-D array; name updated
*                   for clarity.
*/
int UArray2_count(T uarray2)
{
    return uarray2->count;
}


/*
 * UArray2_size: gets the size of the elements in the uarray2 parameter
 * Parameters:   T uarray2: the 2D uarray to query for element size
 * Returns:      int size of the UArray2's elements
 */
int UArray2_size(T uarray2)
{
    return uarray2->size;
}


/*
 * UArray2_at: returns a pointer to the element at the index provided by the 
 *               col and row parameters in the provided uarray2 parameter.
 * Parameters: T uarray2: the 2D array where the desired element is stored
 *             int col: the column index of the desired element
 *             int row: the row index of the desired element
 * Returns:    void *: pointer to the element stored at the provided indices
 * Notes:      col and row are indexed from 0
 *             Raises a CRE if col or row are < 0 or > width and height, 
 *               respectively
 */
void *UArray2_at(T uarray2, int col, int row)
{
    assert(col >= 0 && col < uarray2->width);
    assert(row >= 0 && row < uarray2->height);
    return UArray_at(uarray2->arr, get_array_index(uarray2, col, row));
}


/*
 * get_index: converts a provided (col, row) index into the corresponding index
 *               in a 1D array.
 * Parameters: T uarray2: 2D array where the provided coordinates come from
 *             int col: the column index 
 *             int row: the row index
 * Returns:    int: the equivalent index of the provided coordinates in the 
 *                  underlying 1D uarray
 */ 
int get_array_index(T uarray2, int col, int row)
{
    return col + (row * uarray2->width);
}


/*
 * UArray2_map_row_major: runs apply() function on each element in the uarray2 
 *                        parameter, by row. Includes and optional closure 
 *                        argument pointer (cl). 
 * Parameters:            T uarray2: 2D array on which to perform apply the
 *                          function
 *                        void apply(...): function to apply to provided array
 *                        void *cl: optional closure parameter
 */
void UArray2_map_row_major(T uarray2, void apply(int col, int row, 
                                  T uarray2, void *elemp, void *cl), void *cl)
{
    for (int i = 0; i < uarray2->height; i++) {
        for (int j = 0; j < uarray2->width; j++) {
            apply(j, i, uarray2, UArray2_at(uarray2, j, i), cl);
        }
    }                     
}


/*
 * UArray2_map_col_major: runs apply() function on each element in the uarray2 
 *                        parameter, by column. Includes and optional closure 
 *                        argument pointer (cl).
 * Parameters:            T uarray2: 2D array on which to perform apply the
 *                          function
 *                        void apply(...): function to apply to provided array
 *                        void *cl: optional closure parameter
 */
void UArray2_map_col_major(T uarray2, void apply(int col, int row, 
                                  T uarray2, void *elemp, void *cl), void *cl)
{
    for (int i = 0; i < uarray2->width; i++) {
        for (int j = 0; j < uarray2->height; j++) {
            apply(i, j, uarray2, UArray2_at(uarray2, i, j), cl);
        }
    }  
}


#undef T