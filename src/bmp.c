#include "../include/bmp.h"
#include "../include/helper.h"

/*
 * Initializes |bmpfile| struct with BMP information such as type of header,
 * header length, total file size, etc.
 *
 * This function will also validate that the input file is a valid BMP file.
 *
 * Returns: true if file is supported; false otherwise.
 */
bool init_bmp(FILE * const fp, struct BMP_file * const bmpfile)
{
	unsigned char const expect[] = SUPPORTED_FILE_TYPE;
	unsigned char marker[2];
	struct stat statbuf;

	int fd = fileno(fp);
	if (fd < 0) {
		perror("fileno");
		clean_exit(fp, NULL, EXIT_FAILURE);
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

	bmpfile->tot_size = statbuf.st_size;

	/* Make sure to read from the beginning of the file */
	rewind(fp);
	if (fread(marker, 1, 2, fp) != 2 && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] expect: %s\n", expect); */
	/* printf("[DEBUG] marker: %c%c\n", marker[0], marker[1]); */

	if (memcmp(marker, expect, 2) != 0) {
		fprintf(stderr, "Error: unknown file format\n");
		return false;
	}

	bmpfile->diblen = find_dib_len(fp);
	bmpfile->headerlen = BMPFILEHEADERLEN + bmpfile->diblen;

	switch (bmpfile->diblen) {
	case BITMAPCOREHEADERLEN:
		bmpfile->type = BITMAPCOREHEADER;
		break;
	case OS22XBITMAPHEADERLEN:
		bmpfile->type = OS22XBITMAPHEADER;
		break;
	case BITMAPINFOHEADERLEN:
		bmpfile->type = BITMAPINFOHEADER;
		break;
	case BITMAPV4HEADERLEN:
		bmpfile->type = BITMAPV4HEADER;
		break;
	case BITMAPV5HEADERLEN:
		bmpfile->type = BITMAPV5HEADER;
		break;
	default:
		fprintf(stderr, "Error: unknown DIB header found\n");
		return false;
	}

	bmpfile->data_off = find_data_offset(fp);
	find_bpp(fp, bmpfile);

	return true;
}

/*
 * Finds offset at which the data of BMP file resides. This is where the RGB
 * pixel data is stored.
 * Source: https://en.wikipedia.org/wiki/BMP_file_format#Bitmap_file_header
 *
 * Returns: byte offset where the data portion begins.
 */
size_t find_data_offset(FILE * const fp)
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
size_t find_dib_len(FILE * const fp)
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
void find_bpp(FILE * const fp, struct BMP_file * const bmpfile)
{
	unsigned char buf[4];

	/*
	 * There are two different locations for this value which depend on the
	 * type of header. If the header type is BITMAPCOREHEADER then it's located
	 * in the 24th byte, otherwise, it's the 28th byte.
	 */
	long check_byte = bmpfile->type == BITMAPCOREHEADER ? 24L : 28L;

	if (fseek(fp, check_byte, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (fread(buf, 1, 4, fp) != 4 && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	unsigned int bpp = *((unsigned int *) buf);
	if (bpp != SUPPORTED_BPP) {
		fprintf(stderr,
			"Error: only %u bits per pixel supported, found %u\n",
			SUPPORTED_BPP, bpp);
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	printf("Found bits per pixel: %u\n", bpp);
	bmpfile->bpp = bpp;
}

/*
 * Creates a steganographic BMP file out of |bmpfile->data|. The header for the
 * new BMP file is copied from the source file.
 *
 * Return: file descriptor of new file.
 */
int create_file(FILE * const fp, struct BMP_file * const bmpfile)
{
	int tmpfd;
	unsigned char *header;
	size_t hlen = bmpfile->headerlen;
	char tmpfname[] = "fileXXXXXX";

	if (!(header = malloc(hlen))) {
		perror("malloc");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	printf("Full header (BMP header & DIB header) len: %zu\n",
	       bmpfile->headerlen);

	if ((tmpfd = mkstemp(tmpfname)) < 0) {
		perror("mkstemp");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	rewind(fp);
	if (fread(header, 1, hlen, fp) != hlen && !feof(fp)) {
		perror("fread");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	if (write(tmpfd, header, hlen) < 0) {
		perror("write");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	if (write(tmpfd, bmpfile->data, bmpfile->datalen) < 0) {
		perror("write");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	printf("Created temp file: %s\n", tmpfname);
	free(header);

	return tmpfd;
}

/*
 * Read the RGB pixels (data) of the BMP file.
 * Populates the |bmpfile| struct with the RGB data and the length of the data.
 */
void read_bmp(FILE * const fp, struct BMP_file * const bmpfile)
{
	struct RGB *data;
	size_t rgblen;

	/* Find length of data section */
	if (fseek(fp, 0L, SEEK_END) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] total size: %zu\n", bmpfile->tot_size); */
	/* printf("[DEBUG] data_off:   %zu\n", bmpfile->data_off); */
	if (bmpfile->tot_size <= bmpfile->data_off) {
		fprintf(stderr,
			"Error: file seems to be missing its data section; possibly "
			"corrupt\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	rgblen = bmpfile->tot_size - bmpfile->data_off;

	/* Seek back to beginning of data section */
	if (fseek(fp, (long) bmpfile->data_off, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] rgblen: %zu\n", rgblen); */
	if (!(data = malloc(rgblen))) {
		perror("malloc");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if ((fread(data, 1, rgblen, fp) != rgblen) && !feof(fp)) {
		perror("fread");
		clean_exit(fp, data, EXIT_FAILURE);
	}

	bmpfile->datalen = rgblen;
	bmpfile->data = data;
	printf("Read %zu RGB values\n", rgblen);
}

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(FILE * const fp, struct BMP_file * const bmpfile,
	      char const *msg, size_t const msglen)
{
	size_t maxlen = bmpfile->datalen / 3;

	/* Make sure not to overflow |data| */
	if (msglen > maxlen) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	for (size_t i = 0; i < msglen; i++)
		bmpfile->data[i].b = msg[i];
}

/*
 * Hides |msg| in |data| using LSB method.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg_lsb(FILE * const fp, struct BMP_file * const bmpfile,
		  char const *msg, size_t const msglen)
{
	/*
	 * The reason for the constant 24: with LSB method (using one least
	 * significant bit), each byte from |msg| is spread across each pixel.
	 * Because each of those 8 bits will be spread across 8 (blue) bytes in the
	 * image and there are 3 bytes for every pixel, that makes 24 total bytes.
	 *
	 * Visual representation:
	 *                   Steps                       Bytes used
	 * 1) Store 1st bit from |msg| in 1st blue byte      1
	 * 2) Skip over green, red bytes:                    3
	 * 3) Store 2nd bit from |msg| in 2nd blue byte      4
	 * 4) Skip over green, red bytes:                    6
	 * 5) Store 3rd bit from |msg| in 3rd blue byte      7
	 * 6) Skip over green, red bytes:                    9
	 * 7) Store 4th bit from |msg| in 4th blue byte      10
	 * 8) Skip over green, red bytes:                    12
	 * ....
	 * 15) Store 8th bit from |msg| in 8th blue byte     22
	 * 16) Skip over green, red bytes:                   24
	 */
	size_t maxlen = bmpfile->datalen / 24;

	/* Make sure not to overflow |data| */
	if (msglen > maxlen) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	unsigned char bit;
	unsigned char data;
	size_t m = 0;
	size_t d = 0;
	while (m < msglen && d < maxlen) {
		for (unsigned char j = 0; j < 8; j++) {
			bit = (msg[m] >> j) & 1;
			data = bmpfile->data[d].b;

			if (bit)
				data |= (data & ~(1 << 0)) | (bit << 0);
			else
				data &= (data & ~(1 << 0)) | (bit << 0);

			bmpfile->data[d].b = data;
			d++;
		}

		m++;
	}
}

/*
 * Reveals message hidden within image.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg(struct BMP_file * const bmpfile, size_t const ccount)
{
	size_t len;
	size_t blue_bytes = bmpfile->datalen / 3;

	/* Make sure not to print past the image */
	len = blue_bytes > ccount ? ccount : blue_bytes;

	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 0; i < len; i++) {
		if (isprint(bmpfile->data[i].b))
			printf("%c", bmpfile->data[i].b);
	}
	printf("\nEnd of message\n");
}

/*
 * Reveals message hidden within image using LSB method.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg_lsb(struct BMP_file * const bmpfile, size_t ccount)
{
	/* See comment in hide_msg_lsb() for why constant 24 is used */
	size_t len;
	size_t blue_bytes = bmpfile->datalen / 24;

	/* When using LSB method, the message is stored across 8 bytes */
	ccount *= 8;

	/* Make sure not to print past the image */
	len = blue_bytes > ccount ? ccount : blue_bytes;

	unsigned char buf[8] = { 0 };
	unsigned char data = 0;
	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 0; i < len; i++) {
		buf[i % 8] = (bmpfile->data[i].b >> 0) & 1;

		if ((i % 7) == 0) {
			data = 0;
			for (short j = 7; j >= 0; j--)
				data += (buf[j] << j);

			if (isprint(data))
				printf("%c", data);
		}
	}
	printf("\nEnd of message\n");
}
