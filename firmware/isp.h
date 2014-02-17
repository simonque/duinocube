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

// ISP bootloader for the main Arduino MCU.

#ifndef __ISP_H__
#define __ISP_H__

#include <stdint.h>

// Return codes for ISP.
#define ISP_PROGRAM_SUCCESS                           0
#define ISP_ID_MISMATCH                               1
#define ISP_ERASE_START_ERROR                         2
#define ISP_ERASE_TIMEOUT                             3
#define ISP_FILE_READ_ERROR                           4
#define ISP_WRITE_START_ERROR                         5
#define ISP_WRITE_TIMEOUT                             6
#define ISP_WRITE_VERIFY_ERROR                        7

// Initializes ISP interface.
void isp_init();

// Programs the MCU with program hex code from a file handle.
uint16_t isp_program(uint16_t handle);

#endif  // __ISP_H__
