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

// DuinoCube Core shield library for Arduino.

#include "core.h"

#include <Arduino.h>
#include <SPI.h>

#include "pins.h"

#define WRITE_BIT_MASK      0x80

extern SPIClass SPI;

void DuinoCubeCore::begin() {
  SET_PIN(CORE_SELECT_DIR, OUTPUT);

  // A rising edge on SS resets the SPI interface logic.
  SET_PIN(CORE_SELECT_PIN, LOW);
  SET_PIN(CORE_SELECT_PIN, HIGH);
}

void DuinoCubeCore::writeData(uint16_t addr, const void* data, uint16_t size) {
  SET_PIN(CORE_SELECT_PIN, LOW);

  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(lowByte(addr));

  const uint8_t* data8 = static_cast<const uint8_t*>(data);
  for (const uint8_t* data_end = data8 + size; data8 < data_end; ++data8)
    SPI.transfer(*data8);

  SET_PIN(CORE_SELECT_PIN, HIGH);
}

void DuinoCubeCore::readData(uint16_t addr, void* data, uint16_t size) {
  SET_PIN(CORE_SELECT_PIN, LOW);

  SPI.transfer(highByte(addr) & ~WRITE_BIT_MASK);
  SPI.transfer(lowByte(addr));

  uint8_t* data8 = static_cast<uint8_t*>(data);
  for (uint8_t* data_end = data8 + size; data8 < data_end; ++data8)
    *data8 = SPI.transfer(lowByte(addr));

  SET_PIN(CORE_SELECT_PIN, HIGH);
}

void DuinoCubeCore::writeByte(uint16_t addr, uint8_t data) {
  SET_PIN(CORE_SELECT_PIN, LOW);

  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(lowByte(addr));
  SPI.transfer(data);

  SET_PIN(CORE_SELECT_PIN, HIGH);
}

uint8_t DuinoCubeCore::readByte(uint16_t addr) {
  SET_PIN(CORE_SELECT_PIN, LOW);

  SPI.transfer(highByte(addr) & ~WRITE_BIT_MASK);
  SPI.transfer(lowByte(addr));
  uint8_t result = SPI.transfer(0);

  SET_PIN(CORE_SELECT_PIN, HIGH);

  return result;
}

void DuinoCubeCore::writeWord(uint16_t addr, uint16_t data) {
  SET_PIN(CORE_SELECT_PIN, LOW);

  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(lowByte(addr));
  SPI.transfer(lowByte(data));
  SPI.transfer(highByte(data));

  SET_PIN(CORE_SELECT_PIN, HIGH);
}

uint16_t DuinoCubeCore::readWord(uint16_t addr) {
  SET_PIN(CORE_SELECT_PIN, LOW);

  SPI.transfer(highByte(addr) & ~WRITE_BIT_MASK);
  SPI.transfer(lowByte(addr));
  union {
    uint16_t value_16;
    uint8_t value_8[2];
  };
  value_8[0] = SPI.transfer(0);  // Low byte.
  value_8[1] = SPI.transfer(0);  // High byte.

  SET_PIN(CORE_SELECT_PIN, HIGH);

  return value_16;
}
