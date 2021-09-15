/* comp_img.h
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/21/2021
 * 
 * Contains the interface for working with comp_img (compressed image) structs
 *  including creation, deletion, and printing
 */


#ifndef COMP_IMG_H
#define COMP_IMG_H

#include <stdio.h>
#include <stdint.h>
#include "seq.h"


typedef struct Comp_img *Comp_img;

/* allocates space for a new Comp_img with the provided width and height */
Comp_img Comp_img_new(unsigned width, unsigned height);

/* prints the provided Comp_img to stdout 
   Note: it is a CRE for img to be NULL */
void Comp_img_print(Comp_img img);

/* creates a new Comp_img using data read in from the provided file 
   Note: it is a CRE for fp to be NULL */
Comp_img Comp_img_read(FILE *fp);

/* frees all heap-allocated memory associated with the provided Comp_img 
   Note: it is a CRE for imgp to be NULL */
void Comp_img_free(Comp_img *imgp);

/* returns the width in pixels of the original image 
   Note: it is a CRE for img to be NULL */
unsigned Comp_img_width(Comp_img img);

/* returns the height in pixels of the original image 
   Note: it is a CRE for img to be NULL */
unsigned Comp_img_height(Comp_img img);

/* returns a pointer to the first word in an image's comp_words sequence, 
        removing it from comp_words 
   Note: it is a CRE for img to be NULL */
uint32_t *Comp_img_get_next_word(Comp_img img);

/* adds a pointer to a word to the end of an image's comp_words sequence 
   Note: it is a CRE for img or wordp to be NULL */
void Comp_img_add_word(Comp_img img, uint32_t *wordp);

#endif