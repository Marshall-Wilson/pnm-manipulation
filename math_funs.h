/* math_funs.h
 *
 * Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * 
 * Contains interface for supplemental math functions
 * 
 * Last edited: 3/22/2021
 * 
 */

#ifndef MATH_FUNS_H
#define MATH_FUNS_H

/* If the n < low or n > hi, returns low or hi, otherwise returns n */
float constrain(float n, float low, float hi);

/* Returns n if even, 2(n/2) if odd */
int evenify(int n);

#endif