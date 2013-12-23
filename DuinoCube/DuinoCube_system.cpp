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

// DuinoCube System Shield library for Arduino.

#include <Arduino.h>
#include <SPI.h>

#include "DuinoCube_pins.h"
#include "DuinoCube_rpc.h"

#include "DuinoCube_system.h"

extern SPIClass SPI;

static DuinoCubeRPC rpc;    // For controlling RPC subsystem.

void DuinoCubeSystem::begin() {
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

void DuinoCubeSystem::readSharedRAM(uint16_t addr, void* data, uint16_t size) {
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

void DuinoCubeSystem::writeSharedRAM(
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

