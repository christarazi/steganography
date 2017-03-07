#ifndef _BMP_H_
#define _BMP_H_

#include <stdbool.h>
#include <stdio.h>

#define SUPPORTED_FILE_TYPE     "BM"
#define SUPPORTED_DIBHEAD_SIZE  124U
#define SUPPORTED_BPP           24U
#define SUPPORTED_MIN_FILE_SIZE 138U        /* Header + DIB Header */
#define SUPPORTED_MAX_FILE_SIZE 2147483647U /* 2^31 - 1 == 2GB */

struct RGB {
	unsigned char b;
	unsigned char g;
	unsigned char r;
};

/*
 * Checks the first two bytes of the input file. The first two bytes of a BPM
 * file indicate its type. This function will also try to determine if the
 * file is a valid BPM file by checking it if it meets
 * SUPPORTED_MIN_FILE_SIZE and is less than SUPPORTED_MAX_FILE_SIZE.
 *
 * Returns: true if file is supported; false otherwise.
 */
bool check_bmp(FILE * const fp);

/*
 * Finds offset at which the data of BMP file resides. This is where the RGB
 * pixel data is stored.
 * Source: https://en.wikipedia.org/wiki/BMP_file_format#Bitmap_file_header
 *
 * Returns: byte offset where the data portion begins.
 */
size_t find_data_offset(FILE * const fp);

/*
 * Finds the length of DIB header.
 * Source:
 * https://en.wikipedia.org/wiki/BMP_file_format#DIB_header_.28bitmap_information_header.29
 *
 * Returns: length of DIB header.
 */
size_t find_dib_len(FILE * const fp);

/*
 * Finds the bits per pixel used in BMP file. This value can be found in the
 * 28th byte of the BITMAPINFOHEADER in the DIB header. It is a 4 byte value
 * that is stored unsigned.
 *
 * Returns: bits per pixel in BMP file.
 */
unsigned int find_bpp(FILE * const fp);

/*
 * Create steganographic BMP file with |data|. The header is copied from the
 * source file. This function writes the RGB pixels stored in |data|.
 *
 * Return: file descriptor of created file.
 */
int create_file(FILE * const fp, size_t const hlen,
		const struct RGB *const data, size_t datalen);

/*
 * Read the RGB pixels (data) of the BMP file.
 *
 * The length of the data section in bytes is returned in |rgblen|.
 *
 * Returns: a struct RGB filled with the RGB pixels.
 */
struct RGB *read_bmp(FILE * const fp, size_t offset, size_t *rgblen);

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(FILE * const fp, struct RGB * const data, size_t const rgblen,
		char const *msg, size_t const msglen);

/*
 * Reveals message hidden within image.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg(struct RGB const *data, size_t const rgblen,
		size_t const ccount);

#endif  /* _BMP_H_ */
