/* rgb_to_xyz.c
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/13/2021
 * 
 * Purpose: contains the implementation of functions for converting between
 *              images in the RGB colorspace and images in the CIE XYZ 
 *              colorspace
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assert.h"
#include "rgb_to_xyz.h"
#include "a2methods.h"
#include "a2blocked.h"
#include "a2plain.h"
#include "pnm.h"
#include "mem.h"
#include <math.h>
#include "math_funs.h"



/*********************** Helper function declarations ************************/
XYZ_pix rgb_to_xyz(Pnm_rgb rgb_pix, int denom);

struct Pnm_rgb xyz_to_rgb(XYZ_pix xyz_pix, int denom);

void apply_rgb_to_xyz(int col, int row, A2Methods_UArray2 xyz_array, 
                                        A2Methods_Object *xyz_pix, 
                                        void *rgb_imgp);

void apply_xyz_to_rgb(int col, int row, A2Methods_UArray2 xyz_array, 
                                        A2Methods_Object *xyz_pix, 
                                        void *rgb_imgp);

int xyz_val_to_rgb_val(XYZ_pix xyz_pix, const float Pb_mult, 
                                        const float Pr_mult, int denom);

float rgb_val_to_xyz_val(Pnm_rgb rgb, const float R_mult, const float G_mult, 
                                            const float B_mult, int denom);



/****************************** Constants ************************************/

/* coefficients for conversion from RGB to XYZ */
const float R_TO_Y  =  0.299;       /* R coefficient for Y value */
const float G_TO_Y  =  0.587;       /* G coefficient for Y value */
const float B_TO_Y  =  0.114;       /* B coefficient for Y value */
const float R_TO_PB = -0.168736;    /* R coefficient for Pb value */
const float G_TO_PB = -0.331264;    /* G coefficient for Pb value */
const float B_TO_PB =  0.5;         /* B coefficient for Pb value */
const float R_TO_PR =  0.5;         /* R coefficient for Pr value */
const float G_TO_PR = -0.418688;    /* G coefficient for Pr value */
const float B_TO_PR = -0.081312;    /* B coefficient for Pr value */

/* coefficients for conversion from XYZ to RGB*/
const float PB_TO_R =  0.0;         /* Pb coefficient for R value */
const float PR_TO_R =  1.402;       /* Pr coefficient for R value */
const float PB_TO_G = -0.344136;    /* Pb coefficient for G value */
const float PR_TO_G = -0.714136;    /* Pr coefficient for G value */
const float PB_TO_B =  1.772;       /* Pb coefficient for B value */
const float PR_TO_B =  0.0;         /* Pr coefficient for B value */

/* constraints on values */
const float Y_LOW    =  0.0;        /* lower bound for Y */
const float Y_HI     =  1.0;        /* upper bound for Y */
const float PBPR_LOW = -0.5;        /* lower bound for Pb and Pr */
const float PBPR_HI  =  0.5;        /* upper bound for Pb and Pr */
const float RGB_LOW  =  0.0;        /* lower bound for unscaled RGB values */
const float RGB_HI   =  1.0;        /* upper bound for unscaled RGB values */

/* denominator for decompressed RGB values */
const int DENOMINATOR = 255;



/******************************************************************************
******************************** Functions ************************************
******************************************************************************/

/* rgb_img_to_xyz
 *  Purpose: Given a Pnm_ppm image, creates and returns a copy of the image 
 *           with the rgb values converted to use component video color space 
 *           values.
 *  Parameters: Pnm_ppm rgb_img: the image to be converted
 *  Returns:    XYZ_img: the CIE XYZ version of the provided image
 *  Note:   Images with an odd width or height have their dimensions reduced 
 *              to even numbers.
 *          It is a CRE for rgb_img to have width or height < 2
 *          It is a CRE for rgb_img to be NULL
 */
XYZ_img rgb_img_to_xyz(Pnm_ppm rgb_img)
{
    assert(rgb_img != NULL);
    assert(rgb_img->width > 1 && rgb_img->height > 1);

    XYZ_img xyz_img = XYZ_img_new(evenify(rgb_img->width), 
                                  evenify(rgb_img->height));


    XYZ_img_map(xyz_img, apply_rgb_to_xyz, &rgb_img);       

    return xyz_img;
}


/*xyz_img_to_rgb
 * Purpose:     Converts the provided XYZ_img into an RGB image in a Pnm_ppm
 * Parameters:  XYZ_img xyz_img: the CIE XYZ colorspace image to be converted
 * Returns:     Pnm_ppm: A heap-allocated RGB version of the original image
 * Notes:       It is a CRE for xyz_img to be NULL
 *              It is a CRE for xyz_img to have width or height < 2
 */
Pnm_ppm xyz_img_to_rgb(XYZ_img xyz_img)
{
    assert(xyz_img != NULL);
    assert(XYZ_img_height(xyz_img) > 1 && XYZ_img_width(xyz_img) > 1);

    /* initialize rgb image same size as xyz image */
    Pnm_ppm rgb_img = ALLOC(sizeof(struct Pnm_ppm));
    rgb_img->width = XYZ_img_width(xyz_img);
    rgb_img->height = XYZ_img_height(xyz_img);
    rgb_img->denominator = DENOMINATOR;
    rgb_img->methods = uarray2_methods_plain;
    assert(rgb_img->methods);
    rgb_img->pixels = rgb_img->methods->new(rgb_img->width, rgb_img->height,
                                            sizeof(struct Pnm_rgb));

    /* map through xyz pixels converting to rgb and adding to rgb_img */
    XYZ_img_map(xyz_img, apply_xyz_to_rgb, &rgb_img);
                                                  
    return rgb_img;
}


/* rgb_to_xyz
 *  Purpose:    Converts the provided RGB values into the CIE XYZ color space 
 *  Parameters: Pnm_rgb rgb: The rgb values to convert
 *              int denom: The denominator of the rgb values 
 *  Returns:    XYZ_pix: The equivalent CIE XYZ colorspace values 
 */
XYZ_pix rgb_to_xyz(Pnm_rgb rgb, int denom)
{
    XYZ_pix xyz_pix;
    
    xyz_pix.Y = rgb_val_to_xyz_val(rgb, R_TO_Y, G_TO_Y, B_TO_Y, denom);
    xyz_pix.Y = constrain(xyz_pix.Y, Y_LOW, Y_HI);

    xyz_pix.Pb = rgb_val_to_xyz_val(rgb, R_TO_PB, G_TO_PB, B_TO_PB, denom);
    xyz_pix.Pb = constrain(xyz_pix.Pb, PBPR_LOW, PBPR_HI);

    xyz_pix.Pr = rgb_val_to_xyz_val(rgb, R_TO_PR, G_TO_PR, B_TO_PR, denom);
    xyz_pix.Pr = constrain(xyz_pix.Pr, PBPR_LOW, PBPR_HI);

    return xyz_pix;
}


/* xyz_to_rgb
 *  Purpose:    Given an XYZ struct, returns a Pnm_rgb struct with the 
 *                  converted values
 *  Parameters: XYZ struct to convert
 *  Returns:    Pnm_rgb struct
 */
struct Pnm_rgb xyz_to_rgb(XYZ_pix xyz_pix, int denom)
{
    struct Pnm_rgb rgb_pix;
    
    rgb_pix.red = xyz_val_to_rgb_val(xyz_pix, PB_TO_R, PR_TO_R, denom);
    rgb_pix.green = xyz_val_to_rgb_val(xyz_pix, PB_TO_G, PR_TO_G, denom);
    rgb_pix.blue = xyz_val_to_rgb_val(xyz_pix, PB_TO_B, PR_TO_B, denom);

    return rgb_pix;
}


/* xyz_val_to_rgb_val
 * Purpose:     Calculates and returns the R, G, or B value of an XYZ pixel
 * Parameters:  XYZ_pix: the CIE XYZ values of the pixel
 *              Pb_mult: the coefficient for the Pb value of the pixel
 *              Pr_mult: the coefficient for the Pr value of the pixel
 *              denom:   the denominator to be used when scaling RGB values
 * Returns:     A scaled int representing the relevant R, G, or B value
 * Note:        RGB values are constrained from 0 to denom
 */
int xyz_val_to_rgb_val(XYZ_pix xyz_pix, 
                       const float Pb_mult, const float Pr_mult, int denom)
{
    float n = (1.0 * xyz_pix.Y + Pb_mult * xyz_pix.Pb + Pr_mult * xyz_pix.Pr);
    
    n = constrain(n, RGB_LOW, RGB_HI);

    return (int) floor(n * denom);
}


/* rgb_val_to_xyz_val
 * Purpose:     Calculates and returns the Y, Pb, or Pr values for a provided 
 *                  RGB pixel
 * Parameters:  rgb: the RGB values of the pixel
 *              R_mult: the coefficient for the R value of the pixel
 *              G_mult: the coefficient for the G value of the pixel
 *              B_mult: the coefficient for the B value of the pixel
 *              denom:   the denominator to be used when scaling RGB values
 * Returns:     A float representing the relevant Y, Pb, or PR value 
 */
float rgb_val_to_xyz_val(Pnm_rgb rgb, const float R_mult, const float G_mult, 
                                            const float B_mult, int denom)
{
    /* calculate base value */
    float n = R_mult * rgb->red + G_mult * rgb->green + B_mult * rgb->blue;
    n /= denom;
    
    return n;
}


/* apply_rgb_to_xyz
 *  Purpose:    For the current, empty pixel in an XYZ image, finds the 
 *                  equivalent pixel in the provided RGB image, converts it 
 *                  to CIE XYZ, and stores its value in the pixel 
 *  Parameters: int col, row: The column and row index of the XYZ pixel
 *              xyz_array: The 2D array holding the current XYZ pixel
 *              xyz_pix:   pointer to the current XYZ pixel
 *              rgb_imgp:  pointer to the RGB image being converted(a Pnm_ppm)
 *  Returns:    None
 *  Note:       It is a CRE for rgb_imgp to be NULL
 */
void apply_rgb_to_xyz(int col, int row, A2Methods_UArray2 xyz_array, 
                                        A2Methods_Object *xyz_pix, 
                                        void *rgb_imgp)
{
    assert(rgb_imgp != NULL);
    Pnm_ppm rgb_img = *(Pnm_ppm *)rgb_imgp;
    Pnm_rgb rgb_pix = rgb_img->methods->at(rgb_img->pixels, col, row);

    *(XYZ_pix *)xyz_pix = rgb_to_xyz(rgb_pix, rgb_img->denominator);

    (void) xyz_array;
}


/* apply_xyz_to_rgb
 *  Purpose:    Converts the values of the current pixel in an XYZ_img from 
 *                  CIE XYZ to RGB and stores the resulting value in the 
 *                  equivalent pixel of a provided RGB image 
 *  Parameters: int col, row: The column and row index of the XYZ pixel
 *              xyz_array: The 2D array holding the current XYZ pixel
 *              xyz_pix:   pointer to the current XYZ pixel
 *              rgb_imgp:  pointer to the RGB image being filled (a Pnm_ppm)
 *  Returns:    None
 */
void apply_xyz_to_rgb(int col, int row, A2Methods_UArray2 xyz_array, 
                                        A2Methods_Object *xyz_pix, 
                                        void *rgb_imgp)
{
    assert(rgb_imgp != NULL);
    Pnm_ppm rgb_img = *(Pnm_ppm *)rgb_imgp;
    Pnm_rgb rgb_pix = rgb_img->methods->at(rgb_img->pixels, col, row);

    *rgb_pix = xyz_to_rgb(*(XYZ_pix *)xyz_pix, rgb_img->denominator);
    (void) xyz_array;
}