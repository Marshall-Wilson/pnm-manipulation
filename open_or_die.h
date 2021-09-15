/*
* open_or_die
* Written by: Megan Gelement (mgelem01) and Marshall Wilson (wwilso02)
* Date:       Feburary 21, 2021
* Summary:    Opens file or exits program. Originally written by Megan Gelement
*               and Marshall Wilson for filesnpix on February 15, 2021.
*/

#ifndef OPEN_OR_DIE_H
#define OPEN_OR_DIE_H

#include <stdio.h>
#include <stdlib.h>

FILE *open_or_die(char *file_name);

#endif