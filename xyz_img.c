/* xyz_img.c
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/21/2021
 * 
 * Contains the implementation of functions for working with images in the 
 *  CIE XYZ colorspace
 */


#include <stdio.h>
#include "xyz_img.h"
#include "a2blocked.h"
#include "assert.h"
#include "mem.h"


/* struct XYZ_img
 * Members:     width, height:  the width and height of the image
 *              pixels:         a blocked 2D array with element type struct 
 *                                  XYZ_pix and blocksize 2
 *              methods:        methods to operate on pixels 
 *                                  (always uarray2_methods_blocked)
 */
struct XYZ_img {

    unsigned width, height;
    A2Methods_UArray2 pixels;
    A2Methods_T methods;
};


/* allocates a new XYZ_img with the provided width and height*/
XYZ_img XYZ_img_new(unsigned width, unsigned height)
{
    XYZ_img new_img;
    NEW(new_img);
    new_img->width = width;
    new_img->height = height;
    new_img->methods = uarray2_methods_blocked;
    new_img->pixels = new_img->methods->new_with_blocksize( width, 
                                                            height, 
                                                            sizeof(XYZ_pix), 
                                                            2);
    return new_img;
}

/* frees all heap-allocated memory associated with a XYZ_img 
    Note: it is a CRE for imgp or *imgp to be NULL */
void XYZ_img_free(XYZ_img *imgp)
{
    assert(imgp != NULL);
    assert(*imgp != NULL);
    (*imgp)->methods->free(&(*imgp)->pixels);
    FREE(*imgp);
}


/* returns the width of the provided image 
    Note: it is a CRE for img to be NULL*/
unsigned XYZ_img_width(XYZ_img img)
{
    assert(img != NULL);
    return img->width;
}

/* returns the height of the provided image 
    Note: it is a CRE for img to be NULL*/
unsigned XYZ_img_height(XYZ_img img)
{
    assert(img != NULL);
    return img->height; 
}


/* applies the apply function to each pixel in block major order 
    Note: it is a CRE for img or apply to be NULL*/
void XYZ_img_map(XYZ_img img, A2Methods_applyfun apply, void *cl)
{
    assert(img != NULL);
    assert(apply != NULL);
    img->methods->map_default(img->pixels, apply, cl);
}


/* applies the apply function to each pixel in block major order 
    Note: it is a CRE for img or apply to be NULL */
void XYZ_img_small_map(XYZ_img img, A2Methods_smallapplyfun apply, void *cl)
{
    assert(img != NULL);
    assert(apply != NULL);
    img->methods->small_map_default(img->pixels, apply, cl);
}