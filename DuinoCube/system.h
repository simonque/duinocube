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

// DuinoCube System Shield library for Arduino.

#ifndef __DUINOCUBE_SYSTEM_H__
#define __DUINOCUBE_SYSTEM_H__

#include <stdint.h>

// SPI opcodes.
#define OP_WRITE_COMMAND     1
#define OP_READ_STATUS       2
#define OP_ACCESS_RAM        3
#define OP_RESET             7

// Shared RAM opcodes.
#define RAM_ST_READ          5   // Read/write status register.
#define RAM_ST_WRITE         1
#define RAM_READ             3   // Read/write memory.
#define RAM_WRITE            2

#define RAM_SEQUENTIAL    0x40   // Sets sequential access mode.

class DuinoCubeSystem {
 public:
  static void begin();

  // Serial RAM access functions.
  static void readSharedRAM(uint16_t addr, void* data, uint16_t size);
  static void writeSharedRAM(uint16_t addr, const void* data, uint16_t size);
};

#endif  // __DUINOCUBE_SYSTEM_H__
