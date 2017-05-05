/*
 * Copyright (C) 2017 Chris Tarazi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/args.h"   /* struct Args, parse_args() */
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

	if (!parse_args(argc, argv, &args))
		clean_exit(NULL, NULL, EXIT_FAILURE);

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
