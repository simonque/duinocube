// Copyright (C) 2013 Simon Que
//
// This file is part of DuinoCube.
//
// DuinoCube is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DuinoCube is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DuinoCube.  If not, see <http://www.gnu.org/licenses/>.

// For printf from program memory.

#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdio.h>

#include <avr/pgmspace.h>

#define PRINTF_BUFFER_SIZE  128

// Custom printf and fprintf macros for printing strings from program memory.
#define fprintf(file, str, ...) do { \
    char buf[PRINTF_BUFFER_SIZE]; \
    const char* kStr = str; \
    memcpy_P(buf, kStr, strlen_P(kStr) + 1); \
    fprintf(file, buf, ##__VA_ARGS__); \
  } while (0)

#define printf(str, ...) fprintf_P(stdout, PSTR(str), ##__VA_ARGS__)

#endif  // __PRINTF_H__
