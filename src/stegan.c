#include "../include/stegan.h"

static void hide_msg(struct BMP_file * const bmp, char const *msg,
		     size_t const msglen);
static void hide_msg_lsb(struct BMP_file * const bmp, char const *msg,
			 size_t const msglen);
static void reveal_msg(struct BMP_file * const bmp);
static void reveal_msg_lsb(struct BMP_file * const bmp);
static void hide_file(struct BMP_file * const bmp, char const *hfile);
static void hide_file_lsb(struct BMP_file * const bmp, char const *hfile);
static void reveal_file(struct BMP_file * const bmp);
static void reveal_file_lsb(struct BMP_file * const bmp);

/*
 * This function is the public interface which invokes the appropriate
 * function to perform steganographic hiding.
 */
void hide(struct BMP_file * const bmp, struct Args const * const args)
{
	/* Perform using LSB or simple method */
	bool lsb = (args->mflag && strncmp(args->mmet, "lsb", 3) == 0);

	/* Perform on files or messages */
	bool hidefile = (args->tflag && strncmp(args->ttyp, "file", 4) == 0);

	if (hidefile) {
		lsb ? hide_file_lsb(bmp, args->eval) : hide_file(bmp, args->eval);
	} else {
		lsb ? hide_msg_lsb(bmp, args->eval, args->evallen) :
			hide_msg(bmp, args->eval, args->evallen);
	}

	int const fd = create_bmp(bmp);
	close(fd);
}

/*
 * This function is the public interface which invokes the appropriate
 * function for revealing steganographic data.
 */
void reveal(struct BMP_file * const bmp, struct Args const * const args)
{
	/* Perform using LSB or simple method */
	bool lsb = (args->mflag && strncmp(args->mmet, "lsb", 3) == 0);

	/* Perform on files or messages */
	bool hidefile = (args->tflag && strncmp(args->ttyp, "file", 4) == 0);

	if (hidefile) {
		lsb ? reveal_file_lsb(bmp) : reveal_file(bmp);
	} else {
		lsb ? reveal_msg_lsb(bmp) : reveal_msg(bmp);
	}
}

/*
 * Hides |msg| in |data|.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
static void hide_msg(struct BMP_file * const bmp, char const *msg,
		     size_t const msglen)
{
	size_t maxlimit = (bmp->datalen / 3) - 1;

	/* Make sure not to overflow |bmp->data| */
	if (msglen > maxlimit) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	bmp->data[0].b = (unsigned char) msglen;

	for (size_t i = 0; i < msglen; i++)
		bmp->data[i + 1].b = msg[i];
}

/*
 * Hides |msg| in |data| using LSB method.
 *
 * The message is hidden in the blue channel of the RGB pixel.
 */
static void hide_msg_lsb(struct BMP_file * const bmp,
			 char const *msg, size_t const msglen)
{
	/* The LSB method requires 24 bytes to store the length of the message */
	size_t maxlimit = (bmp->datalen / 3) - 24;

	/* Make sure not to overflow |data| */
	if (msglen > maxlimit) {
		fprintf(stderr, "Error: message is too big for image\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	unsigned char bit;
	unsigned char data;
	size_t m = 0;       /* Index of |msg| */
	size_t d = 0;       /* Index of |bmp->data| */

	/* Write length of message in the first 8 blue bytes */
	for (unsigned char j = 0; j < 8; j++) {
		bit = (msglen >> j) & 1;
		data = bmp->data[d].b;

		/* Change 0th bit (LSB) to |bit| */
		data = (data & ~(1 << 0)) | (bit << 0);

		bmp->data[d].b = data;
		d++;
	}

	while (m < msglen && d < maxlimit) {
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
static void reveal_msg(struct BMP_file * const bmp)
{
	/* Length of message is stored in the first blue byte */
	size_t msglen = (size_t) bmp->data[0].b;
	size_t maxlimit = (bmp->datalen / 3) - 1;

	if (msglen > maxlimit) {
		fprintf(stderr,
			"Error: length mismatch found; possibly corrupt\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	size_t len = msglen + 1;  /* Add one accounting for length byte */
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
static void reveal_msg_lsb(struct BMP_file * const bmp)
{
	/* Length of message is stored in the first 8 blue bytes */
	size_t len = 0;
	size_t maxlimit = (bmp->datalen / 3);
	unsigned char buf[8] = { 0 };
	for (int8_t i = 0; i < 8; i++) {
		buf[i] = (bmp->data[i].b >> 0) & 1;
		len += (buf[i] << i);
	}

	/*
	 * Count the length byte and multiply by 8 to get the get the total number
	 * of bytes the data is spread across in the LSB method.
	 */
	len = (len + 1) * 8;
	if (len > maxlimit) {
		fprintf(stderr,
			"Error: length mismatch found; possibly corrupt\n");
		clean_exit(bmp->fp, bmp->data, EXIT_FAILURE);
	}

	unsigned char data = 0;  /* Byte buffer for reading bits */
	unsigned char count = 0; /* Counter for byte buffer */
	/* printf("[DEBUG] printing %zu bytes\n", len); */
	printf("Message:\n");
	for (size_t i = 8; i < len; i++) {
		buf[i % 8] = (bmp->data[i].b >> 0) & 1;
		count++;

		if (count == 8) {
			count = 0;
			data = 0;
			for (int8_t j = 7; j >= 0; j--)
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
static void hide_file(struct BMP_file * const bmp, char const *hfile)
{
	/*
	 * The maximum amount of bytes that could be written is the number of blue
	 * bytes (hence dividing by 3) and subtract by 4 to account for the length
	 * bytes which represent the size of the file to hide.
	 */
	size_t maxlimit = (bmp->datalen / 3) - 4;

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

	if (hidelen > maxlimit) {
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
 * Hides a file by the name of |hfile| inside image using LSB method.
 *
 * The file is hidden in the blue channel of the RGB pixel.
 */
static void hide_file_lsb(struct BMP_file * const bmp, char const *hfile)
{
	/* Number of blue bytes - 32 bytes to store size of file */
	size_t maxlimit = (bmp->datalen / 3) - (8 * 4);

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

	if (hidelen > maxlimit) {
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

	unsigned char bit;
	unsigned char data;
	size_t h = 0; /* Index for |hdata| */
	size_t d = 0; /* Index for |bmp->data| */

	/* Write size of file in the first 32 blue bytes */
	for (int8_t j = 0; j < 32; j++) {
		bit = (hidelen >> j) & 1;
		data = bmp->data[d].b;

		/* Change 0th bit (LSB) to |bit| */
		data = (data & ~(1 << 0)) | (bit << 0);

		bmp->data[d].b = data;
		d++;
	}

	while (h < hidelen && d < maxlimit) {
		for (int8_t j = 0; j < 8; j++) {
			bit = (hdata[h] >> j) & 1;
			data = bmp->data[d].b;

			/* Change 0th bit (LSB) to |bit| */
			data = (data & ~(1 << 0)) | (bit << 0);

			bmp->data[d].b = data;
			d++;
		}

		h++;
	}

	free(hdata);
	fclose(hfp);
}

/*
 * Reveals file hidden within image.
 *
 * This function does the opposite of hide_file(), but does not alter the
 * |data| values; just creates the revealed file.
 */
static void reveal_file(struct BMP_file * const bmp)
{
	/* Obtain the length of the hidden file by reading the first 4 bytes */
	size_t hidelen = 0;
	for (size_t i = 0; i < 4; i++)
		hidelen += ((unsigned int) (bmp->data[i].b << (8 * i)));

	/* Account for the 4 length bytes */
	size_t maxlimit = (bmp->datalen / 3) - 4;
	if (hidelen > maxlimit) {
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

	close(outfd);
	printf("Successfully decoded file: %s\n", outname);
}

/*
 * Reveals file hidden using the LSB method within image.
 *
 * This function does the opposite of hide_file_lsb(), but does not alter the
 * |data| values; just creates the revealed file.
 */
static void reveal_file_lsb(struct BMP_file * const bmp)
{
	size_t hidelen = 0; /* Size of hidden file */

	/* Obtain size of file is stored in the first 32 blue bytes */
	unsigned char buf[32] = { 0 };
	for (int8_t i = 0; i < 32; ++i) {
		buf[i] = (bmp->data[i].b >> 0) & 1;
		hidelen += ((1 << i) * buf[i]);
	}

	/* Prevent out-of-bounds access to |bmp->data| */
	size_t fullsize = (hidelen + 4) * 8;
	size_t maxlimit = (bmp->datalen / 3);
	if (fullsize > maxlimit) {
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
	 * Begin extracting the hidden file data. Start at offset of 32 because the
	 * first 32 bytes contain the size of the file.
	 */
	size_t h = 0;                 /* Index for |hdata| */
	size_t datalen = hidelen * 8; /* Bytes in |bmp->data| */
	unsigned char count = 0;
	unsigned char data = 0;
	for (size_t i = 0; i < datalen; ++i) {
		buf[i % 8] = (bmp->data[i + 32].b >> 0) & 1;
		count++;

		if (count == 8) {
			count = 0;
			data = 0;
			for (int8_t j = 7; j >= 0; --j)
				data += (buf[j] << j);

			hdata[h++] = data;
		}
	}

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

	close(outfd);
	printf("Successfully decoded file: %s\n", outname);
}
