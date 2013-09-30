// Copyright (C) 2013 Simon Que
//
// This file is part of ChronoCube.
//
// ChronoCube is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ChronoCube is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with ChronoCube.  If not, see <http://www.gnu.org/licenses/>.

// DuinoCube coprocessor definitions.

#ifndef __DEFINES_H__
#define __DEFINES_H__

// NOTE: These must match the definitions in chronocube/core_logic/defines.vh.

// System logic opcodes.
#define OP_NONE                   0
#define OP_READ_COMMAND           1
#define OP_WRITE_STATUS           2
#define OP_ACCESS_RAM             3

// SPI device select.
#define DEV_SELECT_NONE           0
#define DEV_SELECT_LOGIC          1
#define DEV_SELECT_SDCARD         2
#define DEV_SELECT_USB            3
#define DEV_SELECT_FPGA           4
#define DEV_SELECT_FLASH          5

#endif  // __DEFINES_H__