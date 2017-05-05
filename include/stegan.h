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

#ifndef _STEGAN_H_
#define _STEGAN_H_

#include <ctype.h>
#include <stdio.h>

#include "../include/args.h"   /* For struct Args */
#include "../include/bmp.h"    /* For struct BMP_file */
#include "../include/helper.h" /* clean_exit(), read_file(), get_file_size() */

#define SUPPORTED_MAX_MSG_LEN 255

/* Forward declarations */
struct Args;
struct BMP_file;

/*
 * This function is the public interface which invokes the appropriate
 * function to perform steganographic hiding.
 */
void hide(struct BMP_file * const bmp, struct Args const * const args);

/*
 * This function is the public interface which invokes the appropriate
 * function for revealing steganographic data.
 */
void reveal(struct BMP_file * const bmp, struct Args const * const args);

#endif  /* _STEGAN_H_ */
