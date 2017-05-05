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
bool parse_args(int const argc, char * const *argv, struct Args * const args);

#endif  /* _ARGS_H_ */
