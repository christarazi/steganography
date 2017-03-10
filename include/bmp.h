#ifndef _BMP_H_
#define _BMP_H_

#include <stdbool.h>
#include <stdio.h>

#define SUPPORTED_FILE_TYPE     "BM"
#define SUPPORTED_DIBHEAD_SIZE  124U
#define SUPPORTED_BPP           24U
#define SUPPORTED_MIN_FILE_SIZE 26U         /* Smallest possible BMP */
#define SUPPORTED_MAX_FILE_SIZE 2147483647U /* 2^31 - 1 == 2GB */

#define BMPFILEHEADERLEN     14L /* Standard BMP file header */

#define BITMAPCOREHEADERLEN  12L
#define OS22XBITMAPHEADERLEN 64L
#define BITMAPINFOHEADERLEN  40L
#define BITMAPV4HEADERLEN    108L
#define BITMAPV5HEADERLEN    124L

enum DIB_type {
	BITMAPCOREHEADER,
	OS22XBITMAPHEADER,
	/* OS22XBITMAPHEADER-16, (16 byte variant) */
	BITMAPINFOHEADER,
	/* BITMAPV2INFOHEADER, (Undocumented according to Wikipedia) */
	/* BITMAPV3INFOHEADER, (Same as above) */
	BITMAPV4HEADER,
	BITMAPV5HEADER
};

struct BMP_file {
	enum DIB_type type;      /* DIB header type */
	unsigned int  bpp;       /* Bits per pixel */
	size_t        diblen;    /* Length of DIB header */
	size_t        data_off;  /* Offset where RGB pixels begin in the file */
	size_t        datalen;   /* Length in bytes of |data| */
	size_t        headerlen; /* Length in bytes of file header */
	size_t        tot_size;  /* Total size of file in bytes */
	struct RGB    *data;     /* RGB pixels */
};

struct RGB {
	unsigned char b;
	unsigned char g;
	unsigned char r;
};

/*
 * Initializes |bmpfile| struct with BMP information such as type of header,
 * header length, total file size, etc.
 *
 * This function will also validate that the input file is a valid BMP file.
 *
 * Returns: true if file is supported; false otherwise.
 */
bool init_bmp(FILE * const fp, struct BMP_file * const bmpfile);

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
 * Finds the bits per pixel used in BMP file. It is a 4 byte value
 * that is stored unsigned.
 */
void find_bpp(FILE * const fp, struct BMP_file * const bmpfile);

/*
 * Creates a steganographic BMP file out of |bmpfile->data|. The header for the
 * new BMP file is copied from the source file.
 *
 * Return: file descriptor of new file.
 */
int create_file(FILE * const fp, struct BMP_file * const bmpfile);

/*
 * Read the RGB pixels (data) of the BMP file.
 * Populates the |bmpfile| struct with the RGB data and the length of the data.
 */
void read_bmp(FILE * const fp, struct BMP_file * const bmpfile);

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(FILE * const fp, struct BMP_file * const bmpfile,
		char const *msg, size_t const msglen);

/*
 * Reveals message hidden within image.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg(struct BMP_file * const bmpfile, size_t const ccount);

#endif  /* _BMP_H_ */
