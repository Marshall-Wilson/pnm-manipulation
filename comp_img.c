/* comp_img.c
 * by Marshall Wilson (wwilso02) and Eliza Encherman (eenche01)
 * last edited: 3/21/2021
 * 
 * Contains the implementation of functions for interacting with with comp_img 
 * (compressed image) structs including creation, deletion, and printing.
 * 
 *  Note: declaration of struct Comp_img is in comp_img.h
 */

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include "seq.h"
#include "mem.h"
#include "comp_img.h"
#include "bitpack.h"

void print_block(uint32_t *word);

/* struct Comp_img AKA Comp_img
 *  Purpose: stores the data of a compressed ppm image 
 *  Members: int width: the width in pixels of the original image
 *           int height: the height in pixels of the original image
 *           Seq_T comp_words: a Hanson sequence containing the bitpacked data
 *                  for each 2x2 block of pixels in the original image. Words
 *                  are stored in row major order with the first block in the 
 *                  image being the "lowest" block in the sequence
 */
struct Comp_img {
    int width;
    int height;
    Seq_T comp_words;
};


/* Comp_img_new
 * Purpose: Allocates a new comp_img struct with the provided height and width
 *              and allocates a new comp_words sequence for it 
 * Parameters:  unsigned width: the width of the original img 
 *              unsigned height: the height of the original img
 * Returns:     Comp_img: the newly initialized comp_img struct
 * Note:        It is a CRE for width or height to be < 2    
 */
Comp_img Comp_img_new(unsigned width, unsigned height)
{
    assert(width > 1 && height > 1);

    Comp_img new_img;
    NEW(new_img);
    new_img->width = width;
    new_img->height = height;
    int num_words = width * height / 4;
    new_img->comp_words = Seq_new(num_words);

    return new_img;
}


/* Comp_img_free
 * Purpose:     frees all heap-allocated data associated with a comp_img
 * Parameters:  Comp_img *imgp: pointer to the Comp_img to be freed
 * Returns:     None
 * Note:        it is a CRE to pass a null Comp_img *
 */
void Comp_img_free(Comp_img *imgp)
{
    assert(imgp != NULL);
    assert(*imgp != NULL);

    /* free any remaining words */
    while (Seq_length((*imgp)->comp_words) > 0) {
        uint32_t *temp = Seq_remlo((*imgp)->comp_words);
        FREE(temp);
    }

    /* free comp_word sequence */
    Seq_free(&(*imgp)->comp_words);

    /* free struct data */
    FREE(*imgp);
}


/* Comp_img_print
 * Purpose:     Prints the data of a provided image in the Comp40 compressed
 *                  image format 2 format. 
 * Parameters:  Comp_img img: The compressed image to be printed
 * Returns:     None
 * Notes:       Removes and frees each word in the compressed image
 *              It is a CRE for img to be NULL
 */
void Comp_img_print(Comp_img img)
{
    assert(img != NULL);

    printf("COMP40 Compressed image format 2\n%u %u\n", 
                                                    img->width, img->height);

    for (int i = 0, length = Seq_length(img->comp_words); i < length; i ++) {

        uint32_t *curr_word = Seq_remlo(img->comp_words);
        
        print_block(curr_word);

        FREE(curr_word);
    }
}


/* print_block
 * Purpose:     Prints the block data from a 32-bit word in big endian order
 * Parameters:  uint32_t *word: pointer to the 32-bit word to be printed
 * Returns:     None
 */
void print_block(uint32_t *word)
{
    /* prints 32-bit block in big-endian order*/
    for (int i = 3; i >= 0; i--) {
        putchar(Bitpack_getu(*word, 8, i * 8));
    }
}



/* Comp_img_read
 * Purpose:         Creates and returns a new Comp_img using input from the 
 *                      provided stream.
 * Parameters:      FILE *fp: an input stream containing a Comp_img as printed
 *                      by Comp_img_print
 * Returns:         Comp_img: a new Comp_img struct 
 * Note:            it is a CRE for fp to be NULL
 */
Comp_img Comp_img_read(FILE *fp)
{
    assert(fp != NULL);
    
    unsigned height, width;
    int read = fscanf(fp, "COMP40 Compressed image format 2\n%u %u\n", 
                                                            &width, &height); 
    assert(read == 2);
    
    Comp_img compressed = Comp_img_new(width, height);

    uint32_t *word;
    uint8_t c;

    for (int i = 0; i < (int) (width * height / 4); i++) {
        
        word = ALLOC(sizeof(uint32_t));
        for (int j = 3; j >= 0; j--){
            c = getc(fp);
            *word = (uint32_t) Bitpack_newu(*word, 8, 8 * j, (uint64_t) c);  
        }
        Seq_addhi(compressed->comp_words, word);
    }

    return compressed;
}


/* Comp_img_width
 * Purpose:      returns the width in pixels of the original version of the 
 *                  provided image
 * Notes:        it is a CRE for img to be NULL
 */
unsigned Comp_img_width(Comp_img img)
{
    assert(img != NULL);
    return img->width;
}


/* Comp_img_height
 * Purpose:      returns the height in pixels of the original version of the 
 *                  provided image
 * Notes:        it is a CRE for img to be NULL
 */
unsigned Comp_img_height(Comp_img img)
{
    assert(img != NULL);
    return img->height;
}


/* Comp_img_get_next_word
 * Purpose:     Returns a pointer to the next words in the provided image's
 *                  comp_word sequence, removing that word from the sequence
 * Notes:       It is the client's responsibility to free the word  
 *              it is a CRE for img to be NULL
 */
uint32_t *Comp_img_get_next_word(Comp_img img)
{
    assert(img != NULL);
    return Seq_remlo(img->comp_words); 
}


/* Comp_img_add_word
 * Purpose:     adds the provided pointer to a word to the top of the provided
 *                  img's comp_words sequence 
 * Note:        it is a CRE for img to be NULL
 */
void Comp_img_add_word(Comp_img img, uint32_t *wordp)
{
    assert(img != NULL);
    Seq_addhi(img->comp_words, wordp);
}