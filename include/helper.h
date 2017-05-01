#ifndef _HELPER_H_
#define _HELPER_H_

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/bmp.h"    /* For struct RGB */
#include "../include/stegan.h" /* For SUPPORTED_MAX_MSG_LEN */

/* Forward declarations */
struct RGB;

void clean_exit(FILE *fp, struct RGB *rgbs, int const code);

/*
 * Helper function to get the size of the file pointed to by |fp|.
 * The size of the file is passed by reference to |sz|.
 *
 * Returns: true if successful, false otherwise.
 */
bool get_file_size(FILE * const fp, size_t *sz);

/*
 * Helper function to read the data of |hfp|. The length of the data is passed
 * by the parameter |len|, which is just the size of the file determined
 * before a call to this function. The caller must free the data returned and
 * close the FILE pointer handle.
 *
 * Returns: pointer to the data (unsigned char), |hdata| or NULL on error.
 */
unsigned char *read_file(FILE * const hfp, size_t const len);

/*
 * Helper function to perform safe subtraction on unsigned values. The result
 * is stored inside of |r|.
 *
 * Returns: true if subtraction did not underflow, false otherwise.
 */
bool safe_subtract(size_t a, size_t b, size_t *r);

#endif /* _HELPER_H_ */
