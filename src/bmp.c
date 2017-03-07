#include "../include/bmp.h"
#include "../include/helper.h"

/*
 * Checks the first two bytes of the input file. The first two bytes of a BPM
 * file indicate its type. This function will also try to determine if the
 * file is a valid BPM file by checking it if it meets
 * SUPPORTED_MIN_FILE_SIZE and is less than SUPPORTED_MAX_FILE_SIZE.
 *
 * Returns: true if file is supported; false otherwise.
 */
bool check_bmp(FILE * const fp)
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
		fprintf(stderr, "Error: could not extract file info, "
			"make sure it is a regular file\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] size of file: %ld\n", statbuf.st_size); */

	if (statbuf.st_size < SUPPORTED_MIN_FILE_SIZE) {
		fprintf(stderr,
			"Error: file format not supported, possibly corrupt\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (statbuf.st_size > SUPPORTED_MAX_FILE_SIZE) {
		fprintf(stderr, "Error: file too large\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* Make sure to read from the beginning of the file */
	rewind(fp);
	if (fread(marker, 1, 2, fp) != 2 && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] expect: %s\n", expect); */
	/* printf("[DEBUG] marker: %c%c\n", marker[0], marker[1]); */

	return memcmp(marker, expect, 2) == 0;
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
 * Finds the bits per pixel used in BMP file. This value can be found in the
 * 28th byte of the BITMAPINFOHEADER in the DIB header. It is a 4 byte value
 * that is stored unsigned.
 *
 * Returns: bits per pixel in BMP file.
 */
unsigned int find_bpp(FILE * const fp)
{
	unsigned char buf[4];

	if (fseek(fp, 28L, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (fread(buf, 1, 4, fp) != 4 && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	unsigned int bpp = *((unsigned int *) buf);
	printf("Found bits per pixel: %u\n", bpp);

	return bpp;
}

/*
 * Create steganographic BMP file with |data|. The header is copied from the
 * source file. This function writes the RGB pixels stored in |data|.
 *
 * Return: file descriptor of created file.
 */
int create_file(FILE * const fp, size_t const hlen,
		const struct RGB * const data, size_t datalen)
{
	int tmpfd;
	unsigned char *header;
	char tmpfname[] = "fileXXXXXX";

	if (!(header = malloc(hlen))) {
		perror("malloc");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	printf("Full header (BMP header & DIB header) len: %zu\n", hlen);

	if ((tmpfd = mkstemp(tmpfname)) < 0) {
		perror("mkstemp");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	rewind(fp);
	if (fread(header, 1, hlen, fp) != hlen && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (write(tmpfd, header, hlen) < 0) {
		perror("write");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (write(tmpfd, data, datalen) < 0) {
		perror("write");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	printf("Created temp file: %s\n", tmpfname);
	free(header);

	return tmpfd;
}

/*
 * Read the RGB pixels (data) of the BMP file.
 *
 * The length of the data section in bytes is returned in |rgblen|.
 *
 * Returns: a struct RGB filled with the RGB pixels.
 */
struct RGB *read_bmp(FILE * const fp, size_t offset, size_t *rgblen)
{
	struct RGB *data;

	/* Find length of data section */
	if (fseek(fp, 0L, SEEK_END) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	*rgblen = (size_t) ftell(fp) - offset;

	/* Seek back to beginning of data section */
	if (fseek(fp, (long) offset, SEEK_SET) < 0) {
		perror("fseek");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] rgblen: %zu\n", *rgblen); */
	if (!(data = malloc(*rgblen))) {
		perror("malloc");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if ((fread(data, 1, *rgblen, fp) != *rgblen) && !feof(fp)) {
		perror("fread");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	printf("Read %zu RGB values\n", *rgblen);

	return data;
}

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(FILE * const fp, struct RGB * const data, size_t const rgblen,
	      char const *msg, size_t const msglen)
{
	size_t maxlen = rgblen / 3;

	/* Make sure not to overflow |data| */
	if (msglen > maxlen) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(fp, data, EXIT_FAILURE);
	}

	for (size_t i = 0; i < msglen; i++)
		data[i].b = msg[i];
}

/*
 * Reveals message hidden within image.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg(struct RGB const *data, size_t const rgblen,
		size_t const ccount)
{
	size_t len;
	size_t blue_bytes = rgblen / 3;

	/* Make sure not to print past the image */
	len = blue_bytes > ccount ? ccount : blue_bytes;

	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 0; i < len; i++) {
		if (isprint(data[i].b))
			printf("%c", data[i].b);
	}
	printf("\nEnd of message\n");
}
