#include "../include/bmp.h"    /* For manipulating BMP images */
#include "../include/helper.h" /* Helpers, clean_exit(), struct Args */
#include "../include/stegan.h" /* hide_msg(), reveal_msg(), etc. */

int main(int argc, char **argv)
{
	struct Args args = {
		.mflag = false,
		.tflag = false,
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
	bmpfile.fp = fp;
	if (!init_bmp(&bmpfile))
		clean_exit(bmpfile.fp, NULL, EXIT_FAILURE);

	read_bmp(&bmpfile);

	bool lsb = false;
	if (args.mflag && strncmp(args.mmet, "lsb", 3) == 0)
		lsb = true;

	bool filemode = false;
	if (args.tflag && strncmp(args.ttyp, "file", 4) == 0)
		filemode = true;

	if (args.eflag) {
		if (filemode)
			hide_file(&bmpfile, args.eval);
		else {
			if (lsb)
				hide_msg_lsb(&bmpfile, args.eval, args.evallen);
			else
				hide_msg(&bmpfile, args.eval, args.evallen);
		}
		int const fd = create_file(&bmpfile);
		close(fd);
	} else if (args.dflag) {
		if (filemode)
			reveal_file(&bmpfile);
		else {
			if (lsb)
				reveal_msg_lsb(&bmpfile);
			else
				reveal_msg(&bmpfile);
		}
	}

	free(bmpfile.data);
	fclose(bmpfile.fp);
	return EXIT_SUCCESS;
}
