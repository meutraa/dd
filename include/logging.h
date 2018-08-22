/*
 *  This file is part of dd.
 *
 *  Foobar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dd.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Copyright 2018 Paul Meredith
 */

#ifndef DD_LOGGING_H_
#define DD_LOGGING_H_

#include <stdio.h>

#define COLOR_GRAY "\x1b[01;30m"
#define COLOR_RESET "\x1b[0m"

#define info(fmt, ...) \
  if (logging) \
     fprintf(stderr, COLOR_GRAY fmt "\n" COLOR_RESET, __VA_ARGS__);


extern int logging;

void fatal(const char *va, ...);

#endif
