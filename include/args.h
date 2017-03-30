#ifndef _ARGS_H_
#define _ARGS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/helper.h"  /* For clean_exit() */

struct Args {
	bool mflag;           /* -m option */
	bool tflag;           /* -t option */
	bool cflag;           /* -c option */
	bool dflag;           /* -d option */
	bool eflag;           /* -e option */
	size_t evallen;       /* Length of value below */
	char const *mmet;     /* Method passed to -m */
	char const *ttyp;     /* Type passed to -t */
	char const *eval;     /* Value passed to -e */
	char const *bmpfname; /* BMP file name required argument */
};

void print_usage(char const *n);
void parse_args(int const argc, char * const *argv, struct Args * const args);

#endif  /* _ARGS_H_ */
