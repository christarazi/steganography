#ifndef _STEGAN_H_
#define _STEGAN_H_

#include <ctype.h>
#include <stdio.h>

#include "../include/bmp.h"    /* For struct BMP_file */
#include "../include/helper.h" /* clean_exit(), read_file(), get_file_size() */

#define SUPPORTED_MAX_MSG_LEN 255

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(struct BMP_file * const bmp, char const *msg, size_t const msglen);

/*
 * Hides |msg| in |data| using LSB method.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg_lsb(struct BMP_file * const bmp,
		  char const *msg, size_t const msglen);

/*
 * Reveals message hidden within image.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg(struct BMP_file * const bmp);

/*
 * Reveals message hidden within image using LSB method.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg_lsb(struct BMP_file * const bmp);

/*
 * Hides a file by the name of |hfile| inside image.
 *
 * The file is hidden in the blue channel of the RGB pixel.
 */
void hide_file(struct BMP_file * const bmp, char const *hfile);

/*
 * Reveals file hidden within image.
 *
 * This function does the opposite of hide_file(), but does not alter the
 * |data| values; just creates the revealed file.
 */
void reveal_file(struct BMP_file * const bmp);

#endif  /* _STEGAN_H_ */
