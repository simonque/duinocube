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

// DuinoCube Core shield library for Arduino.

#include <Arduino.h>
#include <SPI.h>

#include "DuinoCube_core.h"

#define WRITE_BIT_MASK      0x80

extern SPIClass SPI;

uint8_t DuinoCubeCore::s_ss_pin;

void DuinoCubeCore::begin(uint8_t ss_pin) {
  s_ss_pin = ss_pin;
  pinMode(ss_pin, OUTPUT);

  // A rising edge on SS resets the SPI interface logic.
  digitalWrite(ss_pin, LOW);
  digitalWrite(ss_pin, HIGH);
}

void DuinoCubeCore::writeData(uint16_t addr, const void* data, uint16_t size) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);

  const uint8_t* data8 = static_cast<const uint8_t*>(data);
  for (const uint8_t* data_end = data8 + size; data8 < data_end; ++data8)
    SPI.transfer(*data8);

  digitalWrite(s_ss_pin, HIGH);
}

void DuinoCubeCore::readData(uint16_t addr, void* data, uint16_t size) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr));

  uint8_t* data8 = static_cast<uint8_t*>(data);
  for (uint8_t* data_end = data8 + size; data8 < data_end; ++data8)
    *data8 = SPI.transfer(lowByte(addr));

  digitalWrite(s_ss_pin, HIGH);
}

void DuinoCubeCore::writeByte(uint16_t addr, uint8_t data) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(data);

  digitalWrite(s_ss_pin, HIGH);
}

uint8_t DuinoCubeCore::readByte(uint16_t addr) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr));
  uint8_t result = SPI.transfer(0);

  digitalWrite(s_ss_pin, HIGH);

  return result;
}

void DuinoCubeCore::writeWord(uint16_t addr, uint16_t data) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(lowByte(data));
  SPI.transfer(highByte(data));

  digitalWrite(s_ss_pin, HIGH);
}

uint16_t DuinoCubeCore::readWord(uint16_t addr) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr));
  union {
    uint16_t value_16;
    uint8_t value_8[2];
  };
  value_8[0] = SPI.transfer(0);  // Low byte.
  value_8[1] = SPI.transfer(0);  // High byte.

  digitalWrite(s_ss_pin, HIGH);

  return value_16;
}