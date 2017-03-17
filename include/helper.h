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

struct Args {
	bool mflag;           /* -m option */
	bool cflag;           /* -c option */
	bool dflag;           /* -d option */
	bool eflag;           /* -e option */
	size_t emsglen;       /* Length of value below */
	char const *mmet;     /* Method passed to -m */
	char const *emsg;     /* Message passed to -e */
	char const *bmpfname; /* BMP file name required argument */
};

void print_usage(char const *n);
void clean_exit(FILE *fp, struct RGB *rgbs, int const code);
void parse_args(int const argc, char * const *argv, struct Args * const args);

#endif /* _HELPER_H_ */
