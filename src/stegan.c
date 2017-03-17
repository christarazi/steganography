#include "../include/stegan.h"

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

			/* Change 0th bit (LSB) to |bit| */
			data = (data & ~(1 << 0)) | (bit << 0);

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
	unsigned char count = 0;
	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 0; i < len; i++) {
		buf[i % 8] = (bmpfile->data[i].b >> 0) & 1;
		count++;

		if (count == 8) {
			count = 0;
			data = 0;
			for (short j = 7; j >= 0; j--) {
				data += (buf[j] << j);
			}

			if (isprint(data))
				printf("%c", data);
		}
	}
	printf("\nEnd of message\n");
}

