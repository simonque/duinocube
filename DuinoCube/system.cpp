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

#include <Arduino.h>
#include <SPI.h>

#include "pins.h"
#include "rpc.h"

#include "system.h"

extern SPIClass SPI;

namespace DuinoCube {

static RPC rpc;    // For controlling RPC subsystem.

void System::begin() {
  // Set up the system shield SPI interface.
  SET_PIN(RAM_SELECT_PIN, HIGH);
  SET_PIN(RAM_SELECT_DIR, OUTPUT);

  // Set up the shared RAM for sequential access.
  SET_PIN(RAM_SELECT_PIN, LOW);

  SPI.transfer(RAM_ST_WRITE);
  SPI.transfer(RAM_SEQUENTIAL);

  SET_PIN(RAM_SELECT_PIN, HIGH);

  // Set up the RPC server and client.
  rpc.begin();
}

void System::readSharedRAM(uint16_t addr, void* data, uint16_t size) {
  SET_PIN(RAM_SELECT_PIN, LOW);

  // The SPI RAM uses MSB first mode.
  SPI.transfer(RAM_READ);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));

  char* buf = (char*)data;
  for (uint16_t i = 0; i < size; ++i)
    buf[i] = SPI.transfer(0);

  SET_PIN(RAM_SELECT_PIN, HIGH);
}

void System::writeSharedRAM(
    uint16_t addr, const void* data, uint16_t size) {
  SET_PIN(RAM_SELECT_PIN, LOW);

  // The SPI RAM uses MSB first mode.
  SPI.transfer(RAM_WRITE);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));

  const char* buf = (const char*)data;
  for (uint16_t i = 0; i < size; ++i)
    SPI.transfer(buf[i]);

  SET_PIN(RAM_SELECT_PIN, HIGH);
}

}  // namespace DuinoCube
