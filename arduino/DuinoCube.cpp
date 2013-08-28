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

// DuinoCube library for Arduino.

#include <Arduino.h>
#include <SPI.h>

#include "DuinoCube.h"

#define WRITE_BIT_MASK      0x80

// Default select pins.
#define DEFAULT_SS_PIN      5
#define DEFAULT_SYS_SS_PIN  4

// SPI opcodes.
#define OP_WRITE_COMMAND     1
#define OP_READ_STATUS       2
#define OP_ACCESS_RAM        3
#define OP_RESET             7

#define NUM_RESET_CYCLES     4   // Atmega 328 requires 2.5 us reset pulse.
                                 // At 16 MHz with F = F_osc / 2, that's 2.5
                                 // SPI cycles.

extern SPIClass SPI;

uint8_t DuinoCube::s_ss_pin;
uint8_t DuinoCube::s_sys_ss_pin;

void DuinoCube::begin() {
  begin(DEFAULT_SS_PIN, DEFAULT_SYS_SS_PIN);
}

void DuinoCube::begin(uint8_t ss_pin, uint8_t sys_ss_pin) {
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);

  s_ss_pin = ss_pin;
  pinMode(ss_pin, OUTPUT);

  // A rising edge on SS resets the SPI interface logic.
  digitalWrite(ss_pin, LOW);
  digitalWrite(ss_pin, HIGH);

  // Set up the system shield.
  s_sys_ss_pin = sys_ss_pin;
  digitalWrite(sys_ss_pin, HIGH);
  pinMode(sys_ss_pin, OUTPUT);

  resetRPCServer();
}

void DuinoCube::writeData(uint16_t addr, const void* data, uint16_t size) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);

  const uint8_t* data8 = static_cast<const uint8_t*>(data);
  for (const uint8_t* data_end = data8 + size; data8 < data_end; ++data8)
    SPI.transfer(*data8);

  digitalWrite(s_ss_pin, HIGH);
}

void DuinoCube::readData(uint16_t addr, void* data, uint16_t size) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr));

  uint8_t* data8 = static_cast<uint8_t*>(data);
  for (uint8_t* data_end = data8 + size; data8 < data_end; ++data8)
    *data8 = SPI.transfer(lowByte(addr));

  digitalWrite(s_ss_pin, HIGH);
}

void DuinoCube::writeByte(uint16_t addr, uint8_t data) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(data);

  digitalWrite(s_ss_pin, HIGH);
}

uint8_t DuinoCube::readByte(uint16_t addr) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr));
  uint8_t result = SPI.transfer(0);

  digitalWrite(s_ss_pin, HIGH);

  return result;
}

void DuinoCube::writeWord(uint16_t addr, uint16_t data) {
  digitalWrite(s_ss_pin, LOW);

  SPI.transfer(lowByte(addr));
  SPI.transfer(highByte(addr) | WRITE_BIT_MASK);
  SPI.transfer(lowByte(data));
  SPI.transfer(highByte(data));

  digitalWrite(s_ss_pin, HIGH);
}

uint16_t DuinoCube::readWord(uint16_t addr) {
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

void DuinoCube::resetRPCServer() {
  digitalWrite(s_sys_ss_pin, LOW);
  for (uint8_t i = 0; i < NUM_RESET_CYCLES; ++i)
    SPI.transfer(OP_RESET);
  digitalWrite(s_sys_ss_pin, HIGH);
}

void DuinoCube::writeRPCCommandStatus(uint8_t value) {
  digitalWrite(s_sys_ss_pin, LOW);
  SPI.transfer(OP_WRITE_COMMAND);
  SPI.transfer(value);
  digitalWrite(s_sys_ss_pin, HIGH);
}

uint8_t DuinoCube::readRPCServerStatus() {
  digitalWrite(s_sys_ss_pin, LOW);
  SPI.transfer(OP_READ_STATUS);
  uint8_t result = SPI.transfer(0);
  digitalWrite(s_sys_ss_pin, HIGH);

  return result;
}

int DuinoCube::rpcHello(uint16_t buf_addr) {
  RPC_HelloArgs args;
  args.in.buf_addr = buf_addr;
  int status = rpcExec(RPC_CMD_HELLO, &args, sizeof(args));

  return status;
}

int DuinoCube::rpcInvert(uint16_t buf_addr, uint16_t size) {
  RPC_InvertArgs args;
  args.in.buf_addr = buf_addr;
  args.in.size     = size;
  int status = rpcExec(RPC_CMD_INVERT, &args, sizeof(args));

  return status;
}
