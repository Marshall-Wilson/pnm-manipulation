/* xyz_to_abcd.c
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/22/2021
 * 
 * Purpose: contains the implementation of functions compressing and 
 *              decompressing images in the XYZ CIE colorspace
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "assert.h"
#include "mem.h"
#include "xyz_to_abcd.h"
#include "abcd_to_word.h"
#include "bitpack.h"
#include "rgb_to_xyz.h"

typedef struct Curr_blk Curr_blk;

/* helper function declarations */
void apply_compress_blocks(A2Methods_Object *pixelp, void *curr_blk_cl);
void apply_decomp_blocks(A2Methods_Object *pixelp, void *curr_blk_cl);
float *do_compression_math(float *xyz_val);
float *do_decomp_math(float *abc_val);
Curr_blk new_curr_blk(Comp_img *comp_img);



/* Curr_blk struct
 * Purpose:     used in apply_compress_blocks and apply_decomp_blocks functions
 *              as closure to pass important data about the current block
 *              of pixels and the compressed image
 * Members:     count: which pixel in a 2x2 block is currently being read
 *                             i.e.: 0 | 1
 *                                   2 | 3 
 *              xyz_val: array containing the XYZ values of the 4 pixels
 *                  [Y1, Y2, Y3, Y4, Pb1, Pb2, Pb3, Pb4, Pr1, Pr2, Pr3, Pr4]
 *              word: a pointer to a 64-bit word that will hold a full block's
 *                      data.
 *              compressed: a comp_img struct that will hold the compressed
 *                              image's data as 32 bit words
 */
struct Curr_blk {
    int count;     
    float *xyz_val;   
    uint64_t *word;        
    Comp_img *comp_img;   
};


/* xyz_compress
 * Purpose: Given an XYZ_img, returns a Comp_img struct with the blocks 
 *          compressed into 32-bit words
 * Parameters: The XYZ_img to compress
 * Return:  The Comp_img with the compressed blocks
 * Note: It is a CRE to pass this function a null XYZ_img
 */
Comp_img xyz_compress(XYZ_img xyz_img)
{
    assert(xyz_img != NULL);

    /* Create Comp_img to hold the words to print out */
    Comp_img comp_img = Comp_img_new(XYZ_img_width(xyz_img), 
                                     XYZ_img_height(xyz_img));

    /* Create Curr_blk closure struct for mapping */
    Curr_blk blk;
    blk.count = 0;
    blk.comp_img = &comp_img;
    float xyz_vals[12];
    blk.xyz_val = xyz_vals;

    /* compress the xyz image into the Comp_img format */
    XYZ_img_small_map(xyz_img, apply_compress_blocks, &blk);

    return comp_img;
}


/* xyz_decompress
 *
 * Purpose: Given a Comp_img, returns it as an XYZ_img with Y/Pb/Pr values
 * Parameters: The Comp_img to decompress
 * Return:  The decompressed XYZ_img
 * Note: It is a CRE to pass this function a null Comp_img struct
 */
XYZ_img xyz_decompress(Comp_img comp_img)
{
    assert(comp_img != NULL);
    /* create new xyz_img with correct dimensions */
    XYZ_img xyz_img = XYZ_img_new(Comp_img_width(comp_img), 
                                  Comp_img_height(comp_img));

    /* Create Curr_blk closure struct for mapping */
    Curr_blk blk;
    blk.count = 0;
    blk.comp_img = &comp_img;
    float xyz_vals[12];
    blk.xyz_val = xyz_vals;

    /* map over xyz_img to decompress each block from comp_img */
    XYZ_img_small_map(xyz_img, apply_decomp_blocks, &blk);

    return xyz_img;
}


/* apply_compress_blocks
 * Purpose: collects the data for each 2x2 block of XYZ pixels, compresses them
 *              into a, b, c, d, Pb_avg, Pr_avg format, and packs them into
 *              words in a compressed image struct
 * Parameters:  A2Methods_Object *pixelp: pointer to an XYZ pixel value struct
 *              void *curr_blk_cl: pointer to a Curr_blk which stores info
 *                  about the current block as well as pointers to the
 *                  Comp_img struct we are constructing 
 */
void apply_compress_blocks(A2Methods_Object *pixelp, void *curr_blk_cl)
{
    Curr_blk *blk = (Curr_blk *)curr_blk_cl;
    XYZ_pix pix = *(XYZ_pix *)pixelp;

    /* allocate new word pointer at beginning of new block*/
    if (blk->count == 0) {
        blk->word = ALLOC(sizeof(uint64_t));
    }

    /* retrieve XYZ values for pixel */
    blk->xyz_val[blk->count] = pix.Y;
    blk->xyz_val[blk->count + 4] = pix.Pb;
    blk->xyz_val[blk->count + 8] = pix.Pr;
    
    /* pack values into word at end of each block */
    if (blk->count == 3) {

        /* get compressed values */
        float *abc_val = do_compression_math(blk->xyz_val);
        abcd_to_word(abc_val, blk->word);
        RESIZE(blk->word, sizeof(uint32_t));
        Comp_img_add_word(*blk->comp_img, (uint32_t *)blk->word);
    
        blk->count = 0;
        FREE(abc_val);

    } else {
        blk->count++;
    }
}


/* apply_decomp_blocks
 * Purpose: for block in the image to fill with XYZ pixels, reads and unpacks 
 *              the word and converts a, b, c, d, Pb_avg, Pr_avg into XYZ vals,
 *              then adds the corresponding vals to each pixel
 * Parameters:  A2Methods_Object *pixelp: pointer to an XYZ pixel value struct
 *              void *curr_blk_cl: pointer to a Curr_blk containing info
 *                  about the current block as well as the Comp_img we are
 *                  decompressing from. 
 */
void apply_decomp_blocks(A2Methods_Object *pixelp, void *curr_blk_cl)
{

    Curr_blk *blk = (Curr_blk *)curr_blk_cl;
    XYZ_pix *pix = (XYZ_pix *)pixelp;
   
    /*if start of block and word, grab new word. */
    if (blk->count == 0) {
        
        blk->word = (uint64_t *) Comp_img_get_next_word(*blk->comp_img);
        
        /* Unpack 1 block from word, decompress into array of pixel values */
        float *abc_val = ALLOC(sizeof(float) * 6);
        word_to_abcd(abc_val, blk->word);
        blk->xyz_val = do_decomp_math(abc_val);
    }

    pix->Y = blk->xyz_val[blk->count];
    pix->Pb = blk->xyz_val[blk->count + 4];
    pix->Pr = blk->xyz_val[blk->count + 8];

    /* If last pixel in block, reset pixel count, free the word.*/
    if (blk->count == 3) {
        FREE(blk->word);
        FREE(blk->xyz_val);
        blk->count = 0;
        
    } else {
        blk->count++;
    }

    (void) pix;
}


/*do_compression_math
 * Purpose: Given an array with one block's worth of XYZ values, calculates, 
 *          allocates and returns a pointer to an array with the a, b, c, d 
 *          and average Pb and Pr values
 * Parameters:  an array of floats with the XYZ values of the 4 pixels
 *                  [Y1, Y2, Y3, Y4, Pb1, Pb2, Pb3, Pb4, Pr1, Pr2, Pr3, Pr4]
 * Returns:     a pointer to an array of floats [a, b, c, d, Pb_avg, Pr_avg]
 * Note:        It is a CRE for xyz_val to be NULL
 */
float *do_compression_math(float *xyz_val)
{
    assert(xyz_val != NULL);

    float *abc_val = ALLOC(6 * sizeof(float));
    
    /* calculate a, b, c, d. Store at indices 0-3, respectively */
    abc_val[0] = (xyz_val[3] + xyz_val[2] + xyz_val[1] + xyz_val[0]) / 4.0;
    abc_val[1] = (xyz_val[3] + xyz_val[2] - xyz_val[1] - xyz_val[0]) / 4.0;
    abc_val[2] = (xyz_val[3] - xyz_val[2] + xyz_val[1] - xyz_val[0]) / 4.0;
    abc_val[3] = (xyz_val[3] - xyz_val[2] - xyz_val[1] + xyz_val[0]) / 4.0;
    
    /* calculate average Pb and Pr, store at indices 4 and 5, respectively*/
    float pb_sum = 0.0;
    float pr_sum = 0.0;
    for (int i = 4; i < 8; i++) {
        pb_sum += xyz_val[i]; 
        pr_sum += xyz_val[i + 4];
    }
    abc_val[4] = pb_sum / 4.0;
    abc_val[5] = pr_sum / 4.0;

    return abc_val;
}


/*do_decomp_math
 * Purpose: Given an array with one block's worth of  a, b, c, d and average 
 *          Pb and Pr values, calculates, allocates and returns a pointer to a
 *          float array with the trasformed XYZ (aka Y/Pb/Pr) values
 * Parameters: a pointer to float array abc_val [a, b, c, d, Pb_avg, Pr_avg]
 * Returns: xyz_val, an array of floats with the XYZ values of the 4 pixels
 *          [Y1, Y2, Y3, Y4, Pb1, Pb2, Pb3, Pb4, Pr1, Pr2, Pr3, Pr4]
 * Note:    it is a CRE for abc_val to be NULL
 */
float *do_decomp_math(float *abc_val)
{
    assert(abc_val != NULL);

    float *xyz_val = ALLOC(sizeof(float) * 12);
    
    /* calculate Y values for each pixel */
    xyz_val[0] = abc_val[0] - abc_val[1] - abc_val[2] + abc_val[3];
    xyz_val[1] = abc_val[0] - abc_val[1] + abc_val[2] - abc_val[3];
    xyz_val[2] = abc_val[0] + abc_val[1] - abc_val[2] - abc_val[3];
    xyz_val[3] = abc_val[0] + abc_val[1] + abc_val[2] + abc_val[3];

    /* set Pb and Pr values for each pixel */
    for (int i = 4; i < 8; i++) {
        xyz_val[i] = abc_val[4];
        xyz_val[i + 4] = abc_val[5];   
    }

    FREE(abc_val);
    return xyz_val;
}

