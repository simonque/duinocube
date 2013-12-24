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

// DuinoCube library for the DuinoCube audio/video core.

#ifndef __DUINOCUBE_CORE_H__
#define __DUINOCUBE_CORE_H__

#include <stdint.h>

#include "DuinoCube_defs.h"

enum {
  CORE_BUS_MODE_MAIN,   // Main SPI bus has access to Core.
  CORE_BUS_MODE_ALT,    // Secondary SPI bus has access to Core.
};  // enum

class DuinoCubeCore {
 public:
  // Initialize and teardown functions.
  static void begin();

  // Functions to read/write bytes and words.
  static uint8_t readByte(uint16_t addr);
  static void writeByte(uint16_t addr, uint8_t data);
  static uint16_t readWord(uint16_t addr);
  static void writeWord(uint16_t addr, uint16_t data);

  // Functions to read/write block data.
  static void readData(uint16_t addr, void* data, uint16_t size);
  static void writeData(uint16_t addr, const void* data, uint16_t size);

  // Set bus access to the Core memory space.
  static void setBusMode(uint16_t mode);
};

#endif  // __DUINOCUBE_CORE_H__
