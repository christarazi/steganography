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

	bmpfile->data[0].b = (unsigned char) msglen;

	for (size_t i = 1; i < msglen; i++)
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
	 * The reason for the constant 48: with LSB method (using one least
	 * significant bit), each byte from |msg| is spread across each 8 (blue)
	 * bytes in the image. That means we have to seek through 24 bytes within
	 * the image because we are skipping over the red and green bytes.
	 * Taking into account writing the length byte (also across 8 blue bytes),
	 * that takes the total to 48 bytes.
	 */
	size_t maxlen = bmpfile->datalen / 48;

	/* Make sure not to overflow |data| */
	if (msglen > maxlen) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(fp, bmpfile->data, EXIT_FAILURE);
	}

	unsigned char bit;
	unsigned char data;
	size_t m = 0;
	size_t d = 0;

	/* Write length of message in the first 8 blue bytes */
	for (unsigned char j = 0; j < 8; j++) {
		bit = (msglen >> j) & 1;
		data = bmpfile->data[d].b;

		/* Change 0th bit (LSB) to |bit| */
		data = (data & ~(1 << 0)) | (bit << 0);

		bmpfile->data[d].b = data;
		d++;
	}

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
void reveal_msg(struct BMP_file * const bmpfile)
{
	/* Length of message is stored in the first blue byte */
	size_t len = (size_t) bmpfile->data[0].b;
	len++;

	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 1; i < len; i++) {
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
void reveal_msg_lsb(struct BMP_file * const bmpfile)
{
	/* Length of message is stored in the first 8 blue bytes */
	size_t i;
	size_t len = 0;
	unsigned char buf[8] = { 0 };
	for (i = 0; i < 8; i++) {
		buf[i] = (bmpfile->data[i].b >> 0) & 1;
		len += (buf[i] << i);
	}

	/*
	 * Don't forget to count the length byte.
	 * Multiplying by 8 because that's the number of bytes the data is spread
	 * across with the LSB method.
	 */
	len++;
	len *= 8;
	if (len > (bmpfile->datalen / 3)) {
		fprintf(stderr,
			"Error: length mismatch found; possibly corrupt\n");
		clean_exit(NULL, bmpfile->data, EXIT_FAILURE);
	}

	unsigned char data = 0;
	unsigned char count = 0;
	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (i = 8; i < len; i++) {
		buf[i % 8] = (bmpfile->data[i].b >> 0) & 1;
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
