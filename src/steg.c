#include "../include/bmp.h"    /* For manipulating BMP images */
#include "../include/helper.h" /* Helpers, clean_exit(), struct Args */

int main(int argc, char **argv)
{
	struct Args args = {
		.cflag = false,
		.dflag = false,
		.eflag = false
	};
	parse_args(argc, argv, &args);

	FILE * const fp = fopen(args.bmpfname, "rb");
	if (!fp) {
		perror("fopen");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	if (!check_bmp(fp)) {
		fprintf(stderr, "Error: file format not supported\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	size_t const offset = find_data_offset(fp);
	size_t const diblen = find_dib_len(fp);
	if (diblen != SUPPORTED_DIBHEAD_SIZE) {
		fprintf(stderr, "Error: unsupported file header\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	unsigned int const bpp = find_bpp(fp);
	if (bpp != SUPPORTED_BPP) {
		fprintf(stderr,
			"Error: only %u bits per pixel supported, found %u\n",
			SUPPORTED_BPP, bpp);
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	size_t rgblen;
	struct RGB * const rgbs = read_bmp(fp, offset, &rgblen);

	if (args.eflag) {
		hide_msg(fp, rgbs, rgblen, args.emsg, args.emsglen);
		int const fd = create_file(fp, offset, rgbs, rgblen);
		close(fd);
	} else if (args.dflag) {
		reveal_msg(rgbs, rgblen, args.ccount);
	}

	free(rgbs);
	fclose(fp);
	return EXIT_SUCCESS;
}
