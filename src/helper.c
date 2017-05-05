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

#include "../include/helper.h"

void clean_exit(FILE *fp, struct RGB *rgbs, int const code)
{
	if (fp)
		fclose(fp);
	if (rgbs)
		free(rgbs);
	exit(code);
}

/*
 * Helper function to get the size of the file pointed to by |fp|.
 * The size of the file is passed by reference to |sz|.
 *
 * Returns: true if successful, false otherwise. size of the file.
 */
bool get_file_size(FILE * const fp, size_t *sz)
{
	int fd = fileno(fp);
	if (fd < 0) {
		perror("fileno");
		return false;
	}

	struct stat statbuf;

	if ((fstat(fd, &statbuf) != 0) || (!S_ISREG(statbuf.st_mode))) {
		fprintf(stderr, "Error: could not determine file size, "
			"ensure it is a regular file\n");
		return false;
	}

	*sz = (size_t) statbuf.st_size;
	return true;
}

/*
 * Helper function to read the data of |hfp|. The length of the data is passed
 * by the parameter |len|, which is just the size of the file determined
 * before a call to this function. The caller must free the data returned and
 * close the FILE pointer handle.
 *
 * Returns: pointer to the data (unsigned char), |hdata| or NULL on error.
 */
unsigned char *read_file(FILE * const hfp, size_t const len)
{
	unsigned char *hdata = malloc(len);
	if (!hdata) {
		perror("malloc");
		return NULL;
	}

	if (fread(hdata, 1, len, hfp) != len&& !feof(hfp)) {
		perror("fread");
		free(hdata);
		return NULL;
	}

	return hdata;
}

/*
 * Helper function to perform safe subtraction on unsigned values. The result
 * is stored inside of |r|.
 *
 * Returns: true if subtraction did not underflow, false otherwise.
 */
bool safe_subtract(size_t a, size_t b, size_t *r)
{
	*r = a - b;
	return (a < b) ? false : true;
}
