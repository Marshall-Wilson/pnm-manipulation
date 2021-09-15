/* bitpack.c
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/15/2021
 *
 * Contains the implementations for bitpack.h, which allows for the creation
 *      and manipulation of bitpacked 64-bit words
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "assert.h"
#include "except.h"


/*helper function declarations*/
uint64_t make_mask(unsigned width, unsigned lsb);
int64_t sr_shift(int64_t num, unsigned shift_by);
uint64_t ur_shift(uint64_t num, unsigned shift_by);
uint64_t l_shift(uint64_t num, unsigned shift_by);


/*exception thrown when Bitpack_newX is given a value that does not fit in 
  width bits of the relevant type*/
Except_T Bitpack_Overflow = { "Overflow packing bits" };


/* Bitpack_fitsu
 * Purpose:         checks whether the value of n could be represented by 
 *                      the number of bits in width
 * Parameters:      uint64_t n: the unsigned integer to check
 *                  unsigned width: the number of bits available for n
 * Returns:         bool: true if n can fit in width 
 *                        false otherwise 
 * Note:            The number 0 can fit in 0 bits 
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    return ur_shift(n, width) < 1;
}


/* Bitpack_fitss
 * Purpose:         checks whether the value of n could be represented by 
 *                      the number of bits in width
 * Parameters:      int64_t n: the signed integer to check
 *                  unsigned width: the number of bits available for n
 * Returns:         bool: true if n can fit in width 
 *                        false otherwise 
 * Note:            The number 0 can fit in 0 bits 
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    /* edge case: 0 fits in 0 bits */
    if (width == 0) {
        return n == 0;
    } 
    
    uint64_t unsigned_n;
    if (n < 0) {
        unsigned_n = ~n + 1;
    } else {
        unsigned_n = n;
    }
    return Bitpack_fitsu(unsigned_n, width - 1);
}


/* Bitpack_getu
 * Purpose:         returns the field represented by bits lsb through lsb +
 *                      width in the provided word as an unsigned integer 
 * Parameters:      uint64_t word: the word containing the desired value
 *                  unsigned width: the width of the desired field in bits
 *                  unsigned lsb: the index of the least significant bit of 
 *                      the desired field
 * Returns:         uint64_t: the extracted field as an unsigned int
 * Note:            it is a CRE for width + lsb to be greater than 64 bits
 *                      (error handling done in mask making step)
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    uint64_t value = make_mask(width, lsb) & word;
    return ur_shift(value, lsb);
}


/* Bitpack_gets
 * Purpose:         returns the field represented by bits lsb through lsb +
 *                      width in the provided word as a signed integer 
 * Parameters:      uint64_t word: the word containing the desired value
 *                  unsigned width: the width of the desired field in bits
 *                  unsigned lsb: the index of the least significant bit of 
 *                      the desired field
 * Returns:         int64_t: the extracted field as a signed int
 * Note:            it is a CRE for width + lsb to be greater than 64 bits
 *                      (error handling done in mask making step)
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    int64_t value = make_mask(width, lsb) & word;

    /* l shift to end of word so that negative sign propagates */
    value = (int64_t) l_shift(value, 64 - width - lsb);

    return sr_shift((int64_t) value, 64 - width);
}


/* Bitpack_newu
 * Purpose:         Modifies the field of word starting at index lsb and ending
 *                      at index lsb + width to contain the unsigned integer
 *                      in the value parameter
 * Parameters:      uint64_t word: the word to be modified
 *                  unsigned width: the width of the field to be modified
 *                  unsigned lsb: the index of the least significant bit of 
 *                      the field to be modified
 *                  uint64_t value: the unsigned integer to be inserted
 * Returns:         uint64_t: the modified word
 * Note:            it is a CRE for width + lsb to be greater than 64 bits
 *                      (error handling done in mask making step)
 *                  it is a CRE for value to be too large to fit in width bits
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                                                            uint64_t value)
{
    if (!Bitpack_fitsu(value, width)) {
        RAISE(Bitpack_Overflow);
    }
    /* Makes not-mask, 0s at target field but retains original 1s 
       elsewhere, then fills in 1's from new value at that field*/
    return (word & ~make_mask(width, lsb)) | l_shift(value, lsb);
}


/* Bitpack_news
 * Purpose:         Modifies the field of word starting at index lsb and ending
 *                      at index lsb + width to contain the signed integer
 *                      in the value parameter
 * Parameters:      uint64_t word: the word to be modified
 *                  unsigned width: the width of the field to be modified
 *                  unsigned lsb: the index of the least significant bit of 
 *                      the field to be modified
 *                  int64_t value: the signed integer value to be inserted
 * Returns:         uint64_t: the modified word
 * Note:            it is a CRE for width + lsb to be greater than 64 bits
 *                      (error handling done in mask making step)
 *                  it is a CRE for value to be too large to fit in width bits
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                                                                int64_t value)
{
    if (!Bitpack_fitss(value, width)) {
        RAISE(Bitpack_Overflow);
    }
    return (word & ~make_mask(width, lsb)) | 
                                l_shift(Bitpack_getu(value, width, 0), lsb);
}


/* make_mask
 *  Purpose:        makes a mask of the provided width starting at index lsb
 *  Parameters:     unsigned width: the width of the target field
 *                  unsigned lsb: the index of the smallest bit of the field
 *  Returns:        uint64_t: a mask containing 1s in every bit of the target
 *                      field and 0s in every other bit
 */
uint64_t make_mask(unsigned width, unsigned lsb) 
{
    assert((width + lsb) <= 64);
    uint64_t mask = ~0;
    mask = ur_shift(mask, (64 - width));
    mask = l_shift(mask, lsb);
    return mask;
}


/* ur_shift
 * Purpose:     a modified version of the >> operator for unsigned values. 
 *                  Divides the provided word by 2 to the power of shift_by
 * Parameters:  uint64_t num: the unsigned integer to be shifted
 *              unsigned shift_by: the number of powers of two to shift by
 * Returns:     uint64_t: the resulting shifted number
 * Notes:       it is a CRE for shift_by to be greater than 64
 */
uint64_t ur_shift(uint64_t num, unsigned shift_by)
{
    assert(shift_by <= 64);
    if (shift_by == 64) {
        return (uint64_t) 0;
    }
    return num >> shift_by;
}


/* sr_shift
 * Purpose:     a modified version of the << operator for signed values. 
 *                  Divides the provided word by 2 to the power of shift_by
 * Parameters:  int64_t num: the signed integer to be shifted
 *              unsigned shift_by: the number of powers of two to shift by
 * Returns:     int64_t: the resulting shifted number
 * Notes:       it is a CRE for shift_by to be greater than 64
 */
int64_t sr_shift(int64_t num, unsigned shift_by)
{
    assert(shift_by <= 64);
    if (shift_by == 64) {
        return (int64_t) 0;
    }
    return num >> shift_by;
}


/* l_shift
 * Purpose:     a modified version of the << operator. Divides the provided 
 *                  word by 2 to the power of shift_by
 * Parameters:  uint64_t num: the unsigned integer to be shifted
 *              unsigned shift_by: the number of powers of two to shift by
 * Returns:     uint64_t: the resulting shifted number
 * Notes:       it is a CRE for shift_by to be greater than 64
 */
uint64_t l_shift(uint64_t num, unsigned shift_by)
{
    assert(shift_by <= 64);
    if (shift_by == 64) {
        return (uint64_t) 0;
    }
    return num << shift_by;
}