/* math_funs.c 
 * By Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * Last Edited: 3/21/2021
 * 
 * Contains the implementation for several helpful math functions that are 
 *  used in multiple files 
 */


#include "math_funs.h"
#include <math.h>


/* constrain
 * Purpose:     forces the provided number into the provided range
 * Parameters:  float n: the number to be constrained
 *              float low: the lower bound of the number. If n is lower than 
 *                  this, it is changed to be equal to it
 *              float hi: the upper bound of the number. If n is higher than 
 *                  this, it is changed to be equal to it
 * Returns:     float: the newly constrained number
 */
float constrain(float n, float low, float hi)
{
    if (n > hi) {
        return hi;
    }
    if (n < low) {
        return low;
    }
    return n;
}


/* evenify
 *  Purpose: Returns the provided dimension rounded to the nearest even number 
 *              less than or equal to the original
 */
int evenify(int dimension)
{
    return (dimension / 2) * 2;
}
