/* xyz_to_abcd.h
 * By Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * Last Edited: 3/22/2021
 * 
 * Contains declarations and interface for functions to compress and decompress
 *  images in the CIE XYZ colorspace.
 */


#ifndef XYZ_TO_ABCD_H
#define XYZ_TO_ABCD_H

#include "xyz_img.h"
#include "comp_img.h"


/* Purpose: Given an XYZ_img with CIE XYZ colorspace pixels, returns a
 *              Comp_img struct with the compressed image
 * Note: It is a CRE to pass this function a null XYZ_img
 */
Comp_img xyz_compress(XYZ_img img);

/* Purpose: Given a Comp_img struct with a compressed image, returns it as a 
 *               XYZ_img with CIE XYZ colorspace pixels
 * Note: It is a CRE to pass this function a null Comp_img struct
 */
XYZ_img xyz_decompress(Comp_img img);


#endif