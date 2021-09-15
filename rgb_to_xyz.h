/* rgb_to_xyz.h
 * By Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * Last updated: 3/22/2021
 *
 * Purpose:     Contains declarations and interface for functions to convert
 *                  between images in the RGB colorspace and images in the 
 *                  CIE XYZ colorspace 
 */

#ifndef RGB_TO_XYZ_H
#define RGB_TO_XYZ_H

#include "xyz_img.h"
#include "pnm.h"


/* returns the provided image converted from RGB to CIE XYZ format 
    Note: it is a CRE for rgb_img to be NULL
          it is a CRE for rgb_img to have width or height < 2 */
XYZ_img rgb_img_to_xyz(Pnm_ppm rgb_img);

/* returns the provided image converted from CIE XYZ to RGB format 
    Note: it is a CRE for xyz_img to be NULL
          it is a CRE for xyz_img to have width or height < 2 */
Pnm_ppm xyz_img_to_rgb(XYZ_img xyz_img);

#endif