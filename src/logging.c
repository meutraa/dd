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

#include <stdio.h>
#include <stdarg.h>

#include <stdlib.h>

#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

int logging = 0;

void fatal(const char *va, ...) {
  va_list args;
  va_start(args, va);
  vfprintf(stderr, COLOR_RED "%s\n" COLOR_RESET, args);
  va_end(args);
  exit(EXIT_FAILURE);
}
