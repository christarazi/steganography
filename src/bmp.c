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
		fprintf(stderr, "Error: could not extract file info, "
			"make sure it is a regular file\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	/* printf("[DEBUG] size of file: %ld\n", statbuf.st_size); */

	if (statbuf.st_size < SUPPORTED_MIN_FILE_SIZE) {
		fprintf(stderr,
			"Error: file is too small to be valid BMP file; possibly corrupt\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (statbuf.st_size > SUPPORTED_MAX_FILE_SIZE) {
		fprintf(stderr, "Error: file is too large\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
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
		clean_exit(fp, NULL, EXIT_FAILURE);
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
		fprintf(stderr, "Error: unknown file format\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
		return false;
	}

	bmpfile->data_off = find_data_offset(fp);

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

	*rgblen = (size_t) ftell(fp) - offset;

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
