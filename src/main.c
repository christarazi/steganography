#include "../include/bmp.h"    /* For manipulating BMP images */
#include "../include/helper.h" /* Helpers, clean_exit(), struct Args */
#include "../include/stegan.h" /* hide(), reveal() */

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

	struct BMP_file bmp;
	bmp.fp = fp;
	if (!init_bmp(&bmp))
		clean_exit(bmp.fp, NULL, EXIT_FAILURE);

	read_bmp(&bmp);

	if (args.eflag)
		hide(&bmp, &args);
	else if (args.dflag)
		reveal(&bmp, &args);

	free(bmp.data);
	fclose(bmp.fp);
	return EXIT_SUCCESS;
}
