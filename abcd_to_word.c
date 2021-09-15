/* abcd_to_word.c
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/22/2021
 * 
 * Purpose: contains the implementations for bitpacking a, b, c, d, Pb_avg,
 *          and Pr_avg values into 64-bit words, as well as manipulating and 
 *          unpacking said words
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitpack.h"
#include <math.h>
#include "assert.h"
#include "mem.h"
#include <arith40.h>
#include "math_funs.h"


/* constant widths and lsb's for a, b, c, d, Pb, Pr */
unsigned PBPR_WIDTH = 4;
unsigned BCD_WIDTH = 5;
unsigned A_WIDTH = 9;
unsigned PR_LSB = 0;
unsigned D_LSB = 8;
unsigned A_LSB = 23;

/*helper function declarations*/
int64_t *scale_all_vals(float *abc_val);
void unscale_all_vals(float *abc_val, int64_t *scaled_val);
int64_t scale_bcd(float n);
float unscale_bcd(int64_t n);
uint64_t pack_into_word(int64_t *scaled_val, uint64_t word);
int64_t *unpack_word(uint64_t *word);
int bcd_lsb(int i);


/* abcd_to_word
 * Purpose:     Takes an array of floats representing a, b, c, d, Pb, and PR
 *                  for a 2x2 block of pixels, scales them to small integers, 
 *                  and packs the resulting integers into 32 bits of a 
 *                  provided 64 bit word at LSB = 0
 * Parameters:  float *vals: array of float values [a, b, c, d, Pb, Pr] 
 *              uint64_t *wordp: pointer to the word to be filled
 * Returns:     None
 * Note:        It is a CRE for abc_val or wordp to be NULL.
 */
void abcd_to_word(float *abc_val, uint64_t *wordp)
{
    assert(abc_val != NULL);
    assert(wordp != NULL);
    int64_t *scaled_vals = scale_all_vals(abc_val);

    *wordp = pack_into_word(scaled_vals, *wordp);
    FREE(scaled_vals);
}


/* word_to_abcd
 * Purpose:     Unpacks a provided word into a given array of float values 
 *                  representing a, b, c, d, Pb, and Pr for a block of pixels
 * Parameters:  float *abc_val: the array to be filled 
 *              uint64_t *wordp: pointer to the word to be unpacked
 * Returns:     None
 * Note:        It is a CRE for abc_val or wordp to be NULL.
 */
void word_to_abcd(float *abc_val, uint64_t *wordp)
{
    assert(abc_val != NULL);
    assert(wordp != NULL);
    int64_t *scaled_val = unpack_word(wordp);

    unscale_all_vals(abc_val, scaled_val);
    FREE(scaled_val);
}


/* scale_all_vals
 *  Purpose: Given an array of float values for a, b, c, d, Pb, and Pr, 
 *               allocates and returns a pointer to an array with their 
 *               quantized scaled int64_t forms
 *  Parameters: float *abc_val: array containing [a, b, c, d, Pb, Pr]
 *  Returns:    int64_t *: heap-allocated array with 
 *                  [a, b, c, d, Pb, Pr] as scaled ints
 *  Notes:      it is a CRE for abc_val to be null
 */
int64_t *scale_all_vals(float *abc_val)
{
    int64_t *scaled_val = ALLOC(sizeof(int64_t)*6);

    /* get packed a */
    scaled_val[0] = (int64_t) floor(abc_val[0] * 511);

    /* get packed b, c, d */
    for (int i = 1; i < 4; i++) {
        scaled_val[i] = scale_bcd(abc_val[i]);
    }
    
    /* get packed pb and pr */
    scaled_val[4] = Arith40_index_of_chroma(abc_val[4]);
    scaled_val[5] = Arith40_index_of_chroma(abc_val[5]);
    
    return scaled_val;
}


/* unscale_all_vals
 *  Purpose: Given pointers to an array of scaled int values 
 *               [a, b, c, d, Pb, Pr] and an array of floats, fills the float 
 *               array with the unscaled versions
 *  Parameters: float *abc_val: array where unscaled float values 
 *                      should be stored
 *              int64_t *scaled_val: array containing [a, b, c, d, Pb, Pr]
 *                      in scaled integer form
 *  Returns:    None 
 *  Note:       it is a CRE for abc_val or scaled_val to be NULL
 */
void unscale_all_vals(float *abc_val, int64_t *scaled_val)
{
    assert(abc_val != NULL);
    assert(scaled_val != NULL);

    /* unscale a */
    abc_val[0] = scaled_val[0] / 511.0;

    /* unscale b, c, d */
    for (int i = 1; i < 4; i++) {
        abc_val[i] = unscale_bcd(scaled_val[i]);
    }

    /* unscale pb and pr */
    abc_val[4] = Arith40_chroma_of_index(scaled_val[4]);
    abc_val[5] = Arith40_chroma_of_index(scaled_val[5]);
}


/* scale_bcd
 * Purpose: returns the scaled signed integer form of a provided float 
 *              representing the b, c, or d value for a block of pixels
 * Parameters: float n: the b, c, or d value of a block of pixels
 * Returns:    int64_t: the scaled signed int form of n (between -15 and 15)
 * Note:       quantizes n to a range between -0.3 and 0.3
 */
int64_t scale_bcd(float n)
{
    n *= 50;
    n = constrain(n, -15, 15);    
    return (int64_t) floor(n);
}


/* unscale_bcd
 * Purpose: returns the float form of a provided scaled signed integer
 *              representing the b, c, or d value for a block of pixels
 * Parameters:    int64_t: the scaled signed int form of n (between -15, 15)
 * Returns: float n: the b, c, or d value of a block of pixels
 */
float unscale_bcd(int64_t n)
{
    return n / 50.0;
}


/* pack_into_word
 * Purpose:     Packs a set of integers representing the a, b, c, d, Pb, and 
 *              Pr values of a block of pixels into 32 bits in the designated 
 *              side of a provided 64-bit word. Uses Big Endian order. 
 * Parameters:  int64_t *scaled_val: array containing a, b, c, d, Pb, and Pr
 *              uint64_t *word: pointer to the word to be packed
 * Notes: Assumes the following about the provided vals:
 *          a can be represented by a 9-bit unsigned int
 *          b, c, and d can be represented by a 5-bit signed int
 *          Pb and Pr can be represented by a 4-bit unsigned int
 *        A word packed from the front looks like this:
 *  | unused 32 bits | a (9 bits) | b (5) | c (5) | d (5) | Pb (4) | Pr (4) |
 */
uint64_t pack_into_word(int64_t *scaled_val, uint64_t word)
{
    /* pack Pr/Pb */
    word = Bitpack_newu(word, PBPR_WIDTH, PR_LSB, scaled_val[5]);
    word = Bitpack_newu(word, PBPR_WIDTH, PR_LSB + PBPR_WIDTH, scaled_val[4]);

    /* pack d, c, b */
    for (int i = 3; i > 0; i--) {
        word = Bitpack_news(word, BCD_WIDTH, bcd_lsb(i), scaled_val[i]);
    }
    
    /* pack a */
    word = Bitpack_newu(word, A_WIDTH, A_LSB, scaled_val[0]);
    return word;
}


/* unpack_word
 * Purpose:     given a 32-bit word in a 64-bit word, allocates and returns a 
 *                  pointer to an array with 
 *                  the quantized scaled int representations of 
 *                   [a, b, c, d, Pb, Pr] 
 * Parameters:  uint64_t *word: pointer to the word to be unpacked
 *              bool front_of_word: true if the first 32 bits of the word 
 *                  should be unpacked, false if the last 32 bits should be 
 *                  unpacked 
 * Returns:     int64_t *: heap-allocated array containing the scaled int
 *                   representations of a, b, c, d, Pb, and Pr
 * Note:        it is a CRE for wordp to be NULL
 */
int64_t *unpack_word(uint64_t *wordp)
{
    assert(wordp != NULL);

    int64_t *unpacked_vals = ALLOC(sizeof(int64_t)*6);
    
    /* unpack a */
    unpacked_vals[0] = Bitpack_getu(*wordp, A_WIDTH, A_LSB);

    /* unpack d, c, b */
    for (int i = 3; i > 0; i--) {
        unpacked_vals[i] = Bitpack_gets(*wordp, BCD_WIDTH, bcd_lsb(i));
    }

    /* unpack Pr/Pb */
    unpacked_vals[4] = Bitpack_getu(*wordp, PBPR_WIDTH, PR_LSB + PBPR_WIDTH);
    unpacked_vals[5] = Bitpack_getu(*wordp, PBPR_WIDTH, PR_LSB);
    
    return unpacked_vals;
}


/* bcd_lsb
 * Purpose:     Returns the LSB of b, c, or d in the 32-bit word
 * Parameters:  int i: 0 for b, 1 for c, 2 for d
 */
int bcd_lsb(int i)
{
    return D_LSB + (-BCD_WIDTH * (i - 3));
}

