#ifndef _STEGAN_H_
#define _STEGAN_H_

#include <ctype.h>
#include <stdio.h>

#include "../include/bmp.h"    /* For struct BMP_file */
#include "../include/helper.h" /* clean_exit() */

#define SUPPORTED_MAX_MSG_LEN 255

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(FILE * const fp, struct BMP_file * const bmpfile,
	      char const *msg, size_t const msglen);

/*
 * Hides |msg| in |data| using LSB method.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg_lsb(FILE * const fp, struct BMP_file * const bmpfile,
		  char const *msg, size_t const msglen);

/*
 * Reveals message hidden within image.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg(struct BMP_file * const bmpfile);

/*
 * Reveals message hidden within image using LSB method.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg_lsb(struct BMP_file * const bmpfile);

#endif  /* _STEGAN_H_ */

