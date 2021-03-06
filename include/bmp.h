/*
 * Copyright (C) 2017 Chris Tarazi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
	FILE          *fp;       /* File handle */
	struct RGB    *data;     /* RGB pixels */
};

struct RGB {
	unsigned char b;
	unsigned char g;
	unsigned char r;
};

/*
 * Initializes |bmp| struct with BMP information such as type of header,
 * header length, total file size, etc.
 *
 * This function will also validate that the input file is a valid BMP file.
 *
 * Returns: true if file is supported; false otherwise.
 */
bool init_bmp(struct BMP_file * const bmp);

/*
 * Read the RGB pixels (data) of the BMP file.
 * Populates the |bmp| struct with the RGB data and the length of the data.
 */
void read_bmp(struct BMP_file * const bmp);

/*
 * Creates a steganographic BMP file out of |bmp->data|. The header for the
 * new BMP file is copied from the source file.
 *
 * Return: file descriptor of new file.
 */
int create_bmp(struct BMP_file * const bmp);

#endif  /* _BMP_H_ */
