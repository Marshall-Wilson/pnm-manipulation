

#ifndef UARRAY2REP_INCLUDED
#define UARRAY2REP_INCLUDED
#define T UArray2_T

#include <uarray.h>

struct T {
    int width;    /* number of columns, at least 0 */
    int height;   /* number of rows, at least 0 */
    int count;    /* total number of elements, at least 0 */
    int size;     /* number of bytes in one element */
    UArray_T arr; /* pointer to the 1-D representation of the array */
};

extern void UArray2Rep_init(T uarray2, int width, int height, int size);


#undef T
#endif