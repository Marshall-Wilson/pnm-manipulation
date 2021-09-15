/*
* open_or_die
* Written by: Megan Gelement (mgelem01) and Marshall Wilson (wwilso02)
* Date:       Feburary 21, 2021
* Summary:    Opens file or exits program. Originally written by Megan Gelement
*               and Marshall Wilson for filesnpix on February 15, 2021. 
*/

#include "open_or_die.h"
#include <except.h>

/* Exception */
Except_T Bad_File = { "File could not be opened" };

/*
* open_or_die: opens file or exits program if file cannot be opened.  
* parameters:  string file name. 
* returns:     FILE pointer to open file
*/
FILE *open_or_die(char *file_name) 
{
    FILE *fp = fopen(file_name, "r");
    
    if (fp == NULL) {
        RAISE(Bad_File);
    }

    return fp;
}
