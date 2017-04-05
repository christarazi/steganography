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
