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

#include "../include/bmp.h"
#include "../include/helper.h"

static size_t find_data_offset(FILE * const fp);
static size_t find_dib_len(FILE * const fp);
static void find_bpp(struct BMP_file * const bmp);

/*
 * Initializes |bmp| struct with BMP information such as type of header,
 * header length, total file size, etc.
 *
 * This function will also validate that the input file is a valid BMP file.
 *
 * Returns: true if file is supported; false otherwise.
 */
bool init_bmp(struct BMP_file * const bmp)
{
	unsigned char const expect[] = SUPPORTED_FILE_TYPE;
	unsigned char marker[2];
	struct stat statbuf;

	printf("Validating BMP file...\n");

	int fd = fileno(bmp->fp);
	if (fd < 0) {
		perror("fileno");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	/* Get size of file */
	if ((fstat(fd, &statbuf) != 0) || (!S_ISREG(statbuf.st_mode))) {
		fprintf(stderr, "Error: could not determine file size, "
			"ensure it is a regular file\n");
		return false;
	}

	/* printf("[DEBUG] size of file: %ld\n", statbuf.st_size); */

	if (statbuf.st_size < SUPPORTED_MIN_FILE_SIZE) {
		fprintf(stderr,
			"Error: file is too small to be valid BMP file; possibly corrupt\n");
		return false;
	}

	if (statbuf.st_size > SUPPORTED_MAX_FILE_SIZE) {
		fprintf(stderr, "Error: file is too large\n");
		return false;
	}

	bmp->tot_size = statbuf.st_size;

	/* Make sure to read from the beginning of the file */
	rewind(bmp->fp);
	if (fread(marker, 1, 2, bmp->fp) != 2 && !feof(bmp->fp)) {
		perror("fread");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] expect: %s\n", expect); */
	/* printf("[DEBUG] marker: %c%c\n", marker[0], marker[1]); */

	if (memcmp(marker, expect, 2) != 0) {
		fprintf(stderr, "Error: unknown file format\n");
		return false;
	}

	bmp->diblen = find_dib_len(bmp->fp);
	bmp->headerlen = BMPFILEHEADERLEN + bmp->diblen;

	switch (bmp->diblen) {
	case BITMAPCOREHEADERLEN:
		bmp->type = BITMAPCOREHEADER;
		break;
	case OS22XBITMAPHEADERLEN:
		bmp->type = OS22XBITMAPHEADER;
		break;
	case BITMAPINFOHEADERLEN:
		bmp->type = BITMAPINFOHEADER;
		break;
	case BITMAPV4HEADERLEN:
		bmp->type = BITMAPV4HEADER;
		break;
	case BITMAPV5HEADERLEN:
		bmp->type = BITMAPV5HEADER;
		break;
	default:
		fprintf(stderr, "Error: unknown DIB header found\n");
		return false;
	}

	bmp->data_off = find_data_offset(bmp->fp);
	find_bpp(bmp);
	printf("Done validating BMP file.\n\n");

	return true;
}

/*
 * Read the RGB pixels (data) of the BMP file.
 * Populates the |bmp| struct with the RGB data and the length of the data.
 */
void read_bmp(struct BMP_file * const bmp)
{
	struct RGB *data;
	size_t rgblen;

	/* Find length of data section */
	if (fseek(bmp->fp, 0L, SEEK_END) < 0) {
		perror("fseek");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] total size: %zu\n", bmp->tot_size); */
	/* printf("[DEBUG] data_off:   %zu\n", bmp->data_off); */
	if (bmp->tot_size <= bmp->data_off) {
		fprintf(stderr,
			"Error: file seems to be missing its data section; possibly "
			"corrupt\n");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	rgblen = bmp->tot_size - bmp->data_off;

	/* Seek back to beginning of data section */
	if (fseek(bmp->fp, (long) bmp->data_off, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] rgblen: %zu\n", rgblen); */
	if (!(data = malloc(rgblen))) {
		perror("malloc");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	if ((fread(data, 1, rgblen, bmp->fp) != rgblen) && !feof(bmp->fp)) {
		perror("fread");
		clean_exit(bmp->fp, data, EXIT_FAILURE);
	}

	bmp->datalen = rgblen;
	bmp->data = data;
	printf("Read %zu RGB values from input.\n", rgblen);
}

/*
 * Creates a steganographic BMP file out of |bmp->data|. The header for the
 * new BMP file is copied from the source file.
 *
 * Return: file descriptor of new file.
 */
int create_bmp(struct BMP_file * const bmp)
{
	int tmpfd;
	unsigned char *header;
	size_t hlen = bmp->headerlen;
	char tmpfname[] = "fileXXXXXX";

	if (!(header = malloc(hlen))) {
		perror("malloc");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	if ((tmpfd = mkstemp(tmpfname)) < 0) {
		perror("mkstemp");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	rewind(bmp->fp);
	if (fread(header, 1, hlen, bmp->fp) != hlen && !feof(bmp->fp)) {
		perror("fread");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	if (write(tmpfd, header, hlen) < 0) {
		perror("write");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	if (write(tmpfd, bmp->data, bmp->datalen) < 0) {
		perror("write");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	printf("Created steganographic file: %s\n", tmpfname);
	free(header);

	return tmpfd;
}

/*
 * Finds offset at which the data of BMP file resides. This is where the RGB
 * pixel data is stored.
 * Source: https://en.wikipedia.org/wiki/BMP_file_format#Bitmap_file_header
 *
 * Returns: byte offset where the data portion begins.
 */
static size_t find_data_offset(FILE * const fp)
{
	unsigned char addr[4];

	/* Seek to 10th byte containing address of data section (4 bytes) */
	if (fseek(fp, 10L, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (fread(addr, 1, 4, fp) != 4 && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* BMP files are in little-endian */
	printf("Found address of data section: [0x%02x%02x%02x%02x]\n",
	       addr[3], addr[2], addr[1], addr[0]);

	size_t offset = (size_t) *((unsigned int *) addr);

	return offset;
}

/*
 * Finds the length of DIB header.
 * Source:
 * https://en.wikipedia.org/wiki/BMP_file_format#DIB_header_.28bitmap_information_header.29
 *
 * Returns: length of DIB header.
 */
static size_t find_dib_len(FILE * const fp)
{
	unsigned char buf[4];

	if (fseek(fp, 14L, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (fread(buf, 1, 4, fp) != 4 && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	size_t len = (size_t) *((unsigned int *) buf);
	printf("Found DIB header len: %zu\n", len);

	return len;
}

/*
 * Finds the bits per pixel used in BMP file. It is a 4 byte value
 * that is stored unsigned.
 */
static void find_bpp(struct BMP_file * const bmp)
{
	unsigned char buf[4];

	/*
	 * There are two different locations for this value which depend on the
	 * type of header. If the header type is BITMAPCOREHEADER then it's located
	 * in the 24th byte, otherwise, it's the 28th byte.
	 */
	long check_byte = bmp->type == BITMAPCOREHEADER ? 24L : 28L;

	if (fseek(bmp->fp, check_byte, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	if (fread(buf, 1, 4, bmp->fp) != 4 && !feof(bmp->fp)) {
		perror("fread");
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	unsigned int bpp = *((unsigned int *) buf);
	if (bpp != SUPPORTED_BPP) {
		fprintf(stderr,
			"Error: only %u bits per pixel supported, found %u\n",
			SUPPORTED_BPP, bpp);
		clean_exit(bmp->fp, NULL, EXIT_FAILURE);
	}

	printf("Found bits per pixel: %u\n", bpp);
	bmp->bpp = bpp;
}
