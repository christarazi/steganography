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

	struct BMP_file bmpfile;
	if (!init_bmp(fp, &bmpfile)) {
		fprintf(stderr, "Error: file format not supported\n");
		clean_exit(fp, NULL, EXIT_FAILURE);
	}

	read_bmp(fp, &bmpfile);

	if (args.eflag) {
		hide_msg(fp, &bmpfile, args.emsg, args.emsglen);
		int const fd = create_file(fp, &bmpfile);
		close(fd);
	} else if (args.dflag) {
		reveal_msg(&bmpfile, args.ccount);
	}

	free(bmpfile.data);
	fclose(fp);
	return EXIT_SUCCESS;
}
