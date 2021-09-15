/* compress40.c
 * By Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * 
 *  Contains the implementation and definitions for compress40.h, which
 *      controls the compression and decompression of ppm images into and from
 *      the Comp 40 Compressed Image Format 2
 * 
 * Last updated 3/22/2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "compress40.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "rgb_to_xyz.h"
#include "xyz_to_abcd.h"
#include "xyz_img.h"
#include "comp_img.h"

/* Helper Function */
void free_all_img(Pnm_ppm rgb_img, XYZ_img xyz_img, Comp_img compressed_img);


/* compress40
 * Purpose:     Given a ppm image, prints the compressed image to stdout
 * Parameters:  The filestream of the ppm
 * Returns:     N/A
 * Note:        It is a CRE for input to be NULL
 */
extern void compress40  (FILE *input)
{
    assert(input != NULL);

    /* set UArray2 methods to plain for the initial read */
    A2Methods_T input_methods = uarray2_methods_plain; 
    assert(input_methods);

    /* read rgb img, convert to XYZ img, compress into Comp_img, and print */
    Pnm_ppm rgb_img = Pnm_ppmread(input, input_methods);
    A2Methods_UArray2 xyz_img = rgb_img_to_xyz(rgb_img);
    Comp_img compressed_img = xyz_compress(xyz_img);
    Comp_img_print(compressed_img);

    free_all_img(rgb_img, xyz_img, compressed_img);
}


/* decompress40
 * Purpose:     Given a compressed image, prints the decompressed ppm to stdout
 * Parameters:  The filestream of the compressed image
 * Returns:     N/A
 * Note:        It is a CRE for input to be NULL
 */
extern void decompress40(FILE *input)
{
    assert(input != NULL);

    /* Read Comp_img, decompress into XYZ img, convert to RGB img, and print */
    Comp_img compressed_img = Comp_img_read(input);
    A2Methods_UArray2 xyz_img = xyz_decompress(compressed_img);
    Pnm_ppm rgb_img = xyz_img_to_rgb(xyz_img);
    Pnm_ppmwrite(stdout, rgb_img);

    free_all_img(rgb_img, xyz_img, compressed_img);
}


/* free_all_img
 * Purpose:     Given a Pnm_ppm, XYZ_img, and Comp_img, frees them
 * Note:        It is a CRE for any img to be NULL
 */
void free_all_img(Pnm_ppm rgb_img, XYZ_img xyz_img, Comp_img compressed_img)
{
    Comp_img_free(&compressed_img);
    XYZ_img_free(&xyz_img);
    Pnm_ppmfree(&rgb_img);
}
