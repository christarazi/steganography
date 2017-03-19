#include "../include/stegan.h"

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg(struct BMP_file * const bmp, char const *msg, size_t const msglen)
{
	size_t maxlen = bmp->datalen / 3;

	/* Make sure not to overflow |data| */
	if (msglen > maxlen) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	bmp->data[0].b = (unsigned char) msglen;

	for (size_t i = 1; i < msglen; i++)
		bmp->data[i].b = msg[i];
}

/*
 * Hides |msg| in |data| using LSB method.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
void hide_msg_lsb(struct BMP_file * const bmp,
		  char const *msg, size_t const msglen)
{
	/*
	 * The reason for the constant 48: with LSB method (using one least
	 * significant bit), each byte from |msg| is spread across each 8 (blue)
	 * bytes in the image. That means we have to seek through 24 bytes within
	 * the image because we are skipping over the red and green bytes.
	 * Taking into account writing the length byte (also across 8 blue bytes),
	 * that takes the total to 48 bytes.
	 */
	size_t maxlen = bmp->datalen / 48;

	/* Make sure not to overflow |data| */
	if (msglen > maxlen) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	unsigned char bit;
	unsigned char data;
	size_t m = 0;
	size_t d = 0;

	/* Write length of message in the first 8 blue bytes */
	for (unsigned char j = 0; j < 8; j++) {
		bit = (msglen >> j) & 1;
		data = bmp->data[d].b;

		/* Change 0th bit (LSB) to |bit| */
		data = (data & ~(1 << 0)) | (bit << 0);

		bmp->data[d].b = data;
		d++;
	}

	while (m < msglen && d < maxlen) {
		for (unsigned char j = 0; j < 8; j++) {
			bit = (msg[m] >> j) & 1;
			data = bmp->data[d].b;

			/* Change 0th bit (LSB) to |bit| */
			data = (data & ~(1 << 0)) | (bit << 0);

			bmp->data[d].b = data;
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
void reveal_msg(struct BMP_file * const bmp)
{
	/* Length of message is stored in the first blue byte */
	size_t len = (size_t) bmp->data[0].b;
	len++;

	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 1; i < len; i++) {
		if (isprint(bmp->data[i].b))
			printf("%c", bmp->data[i].b);
	}
	printf("\nEnd of message\n");
}

/*
 * Reveals message hidden within image using LSB method.
 *
 * This function does the opposite of hide_msg(), but does not alter the
 * |data| values; just prints the message.
 */
void reveal_msg_lsb(struct BMP_file * const bmp)
{
	/* Length of message is stored in the first 8 blue bytes */
	size_t i;
	size_t len = 0;
	unsigned char buf[8] = { 0 };
	for (i = 0; i < 8; i++) {
		buf[i] = (bmp->data[i].b >> 0) & 1;
		len += (buf[i] << i);
	}

	/*
	 * Don't forget to count the length byte.
	 * Multiplying by 8 because that's the number of bytes the data is spread
	 * across with the LSB method.
	 */
	len++;
	len *= 8;
	if (len > (bmp->datalen / 3)) {
		fprintf(stderr,
			"Error: length mismatch found; possibly corrupt\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	unsigned char data = 0;
	unsigned char count = 0;
	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (i = 8; i < len; i++) {
		buf[i % 8] = (bmp->data[i].b >> 0) & 1;
		count++;

		if (count == 8) {
			count = 0;
			data = 0;
			for (short j = 7; j >= 0; j--)
				data += (buf[j] << j);

			if (isprint(data))
				printf("%c", data);
		}
	}
	printf("\nEnd of message\n");
}

/*
 * Hides a file by the name of |hfile| inside image.
 *
 * The file is hidden in the blue channel of the RGB pixel.
 */
void hide_file(struct BMP_file * const bmp, char const *hfile)
{
	/*
	 * The maximum amount of bytes that could be written is the number of blue
	 * bytes (hence dividing by 3) and subtract by 4 to account for the length
	 * bytes which represent the size of the file to hide.
	 */
	size_t maxlen = (bmp->datalen / 3) - 4;

	FILE *hfp = fopen(hfile, "rb");
	if (!hfp) {
		perror("fopen");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	size_t hidelen;
	if (!get_file_size(hfp, &hidelen)) {
		fprintf(stderr, "Error: could not obtain size of file\n");
		fclose(hfp);
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	if (hidelen > maxlen) {
		fprintf(stderr, "Error: file too large to hide inside image\n");
		fclose(hfp);
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	unsigned char *hdata = read_file(hfp, hidelen);
	if (!hdata) {
		fprintf(stderr, "Error: could not read file\n");
		fclose(hfp);
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	/* Write size of file (4 bytes) */
	for (size_t i = 0; i < 4; i++)
		bmp->data[i].b = ((unsigned char) (hidelen >> (8 * i)));

	/*
	 * Replace bitmap file data with the file data to hide.
	 * Start at offset of 4 because of the 4 length bytes.
	 */
	for (size_t i = 0; i < hidelen; i++)
		bmp->data[i + 4].b = hdata[i];

	free(hdata);
	fclose(hfp);
}

/*
 * Reveals file hidden within image.
 *
 * This function does the opposite of hide_file(), but does not alter the
 * |data| values; just creates the revealed file.
 */
void reveal_file(struct BMP_file * const bmp)
{
	/* Obtain the length of the hidden file by reading the first 4 bytes */
	size_t hidelen = 0;
	for (size_t i = 0; i < 4; i++)
		hidelen += ((unsigned int) (bmp->data[i].b << (8 * i)));

	/* See comment in hide_file() for explanation of the below calculation */
	size_t maxlen = (bmp->datalen / 3) - 4;

	if (hidelen > maxlen) {
		fprintf(stderr,
			"Error: length mismatch found; possibly corrupt\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	unsigned char *hdata = malloc(hidelen);
	if (!hdata) {
		perror("malloc");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	/*
	 * Begin extracting the hidden file data. Start at offset of 4 because the
	 * 4 length bytes at the beginning.
	 */
	for (size_t i = 0; i < hidelen; i++)
		hdata[i] = bmp->data[i + 4].b;

	char outname[] = "outXXXXXX";
	int outfd = mkstemp(outname);
	if (outfd < 0) {
		perror("mkstemp");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	if (write(outfd, hdata, hidelen) < 0) {
		perror("write");
		close(outfd);
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	printf("Decoded file: %s\n", outname);
}
