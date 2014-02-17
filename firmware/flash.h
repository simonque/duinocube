// Copyright (C) 2014 Simon Que
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

// DuinoCube FPGA flash reprogram functions.

#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

// Return codes for flash programming.
#define FLASH_PROGRAM_SUCCESS                           0
#define FLASH_PROGRAM_ID_MISMATCH                       1
#define FLASH_PROGRAM_ERASE_START_ERROR                 2
#define FLASH_PROGRAM_ERASE_TIMEOUT                     3
#define FLASH_PROGRAM_FILE_READ_ERROR                   4
#define FLASH_PROGRAM_WRITE_START_ERROR                 5
#define FLASH_PROGRAM_WRITE_TIMEOUT                     6
#define FLASH_PROGRAM_WRITE_VERIFY_ERROR                7

// Initializes flash interface.
void flash_init();

// Re-programs flash with data from a file handle.
uint16_t flash_reprogram(uint16_t handle);

#endif  // __FLASH_H__
