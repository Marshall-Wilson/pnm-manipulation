/* xyz_img.h
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/21/2021
 * 
 * Contains the interface for working with images in the CIE XYZ colorspace
 */

#ifndef XYZ_IMG_H
#define XYZ_IMG_H

#include "a2methods.h"
#include "pnm.h"


/* CIE XYZ colorspace pixel values */
typedef struct XYZ_pix {
    float Y, Pb, Pr;
} XYZ_pix;


typedef struct XYZ_img *XYZ_img;


/* allocates a new XYZ_img width the provided width and height*/
XYZ_img XYZ_img_new(unsigned width, unsigned height);

/* frees all heap-allocated memory associated with a XYZ_img 
    Note: it is a CRE for imgp or *imgp to be NULL*/
void XYZ_img_free(XYZ_img *imgp);

/* returns the width of the provided image 
    Note: it is a CRE for img to be NULL*/
unsigned XYZ_img_width(XYZ_img img);

/* returns the height of the provided image 
    Note: it is a CRE for img to be NULL*/
unsigned XYZ_img_height(XYZ_img img);

/* applies the apply function to each pixel in block major order 
    Note: it is a CRE for img or apply to be NULL*/
void XYZ_img_map(XYZ_img img, A2Methods_applyfun apply, void *cl);

/* applies the small apply function to each pixel in block major order
    Note: it is a CRE for img or apply to be NULL*/
void XYZ_img_small_map(XYZ_img img, A2Methods_smallapplyfun apply, void *cl);


#endif