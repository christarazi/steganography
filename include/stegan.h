#ifndef _STEGAN_H_
#define _STEGAN_H_

#include <ctype.h>
#include <stdio.h>

#include "../include/bmp.h"    /* For struct BMP_file */
#include "../include/helper.h" /* clean_exit(), read_file(), get_file_size() */

#define SUPPORTED_MAX_MSG_LEN 255

/* Forward declarations */
struct BMP_file;
struct Args;

/*
 * This function is the public interface which invokes the appropriate
 * function to perform steganographic hiding.
 */
void hide(struct BMP_file * const bmp, struct Args const * const args);

/*
 * This function is the public interface which invokes the appropriate
 * function for revealing steganographic data.
 */
void reveal(struct BMP_file * const bmp, struct Args const * const args);

#endif  /* _STEGAN_H_ */
