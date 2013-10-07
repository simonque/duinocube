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

// Custom printf definition file.

#ifndef __PRINTF_F__
#define __PRINTF_F__

#include <stdio.h>

#include <avr/pgmspace.h>

// This is the RAM buffer into which strings are copied from program memory.
extern char printf_buffer[];

// Custom printf and fprintf macros for printing strings from program memory.
#define fprintf_P(file, str, ...) { \
    memcpy_PF(printf_buffer, (uint_farptr_t)str, \
              strlen_PF((uint_farptr_t)str) + 1); \
    fprintf(file, printf_buffer, ##__VA_ARGS__); \
  }

#define printf_P(str, ...) fprintf_P(stdout, str, ##__VA_ARGS__)

#endif  // __PRINTF_F__
