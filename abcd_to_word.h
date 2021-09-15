/* abcd_to_word.h
 *
 * By Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * Last Edited: 3/17/2021
 * 
 * Purpose: contains the interface for bitpacking a, b, c, d, Pb_avg, and 
 *          Pr_avg values into 64-bit words, as well as manipulating and 
 *          unpacking said words
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifndef ABCD_TO_WORD_H
#define ABCD_TO_WORD_H


/* Purpose: packs the provided array of block values 
 *          [a, b, c, d, Pb_avg, Pr_avg] into the provided word
 *
 * Note:    It is a CRE for abc_val or wordp to be NULL.
 */
void abcd_to_word(float *abc_val, uint64_t *wordp);


/* Purpose: unpacks the provided word into an array of block values 
 *          [a, b, c, d, Pb_avg, Pr_avg]
 *
 * Note:   It is a CRE for abc_val or wordp to be NULL.
 */
void word_to_abcd(float *abc_val, uint64_t *wordp);

#endif