#include "../include/bmp.h"    /* For manipulating BMP images */
#include "../include/helper.h" /* Helpers, clean_exit(), struct Args */
#include "../include/stegan.h" /* hide_msg(), reveal_msg(), etc. */

int main(int argc, char **argv)
{
	struct Args args = {
		.mflag = false,
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
	if (!init_bmp(fp, &bmpfile))
		clean_exit(fp, NULL, EXIT_FAILURE);

	read_bmp(fp, &bmpfile);

	bool lsb = false;
	if (args.mflag && strncmp(args.mmet, "lsb", 3) == 0)
		lsb = true;

	if (args.eflag) {
		if (lsb)
			hide_msg_lsb(fp, &bmpfile, args.emsg, args.emsglen);
		else
			hide_msg(fp, &bmpfile, args.emsg, args.emsglen);

		int const fd = create_file(fp, &bmpfile);
		close(fd);
	} else if (args.dflag) {
		if (lsb)
			reveal_msg_lsb(&bmpfile);
		else
			reveal_msg(&bmpfile);
	}

	free(bmpfile.data);
	fclose(fp);
	return EXIT_SUCCESS;
}
