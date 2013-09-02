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

// DuinoCube RPC subsystem library for Arduino.

#include <Arduino.h>
#include <SPI.h>

#include "DuinoCube_system.h"

#include "DuinoCube_rpc.h"

#define NUM_RESET_CYCLES     4   // Atmega 328 requires 2.5 us reset pulse.
                                 // At 16 MHz with F = F_osc / 2, that's 2.5
                                 // SPI cycles.

extern SPIClass SPI;

// For accessing other DuinoCube system shield functions.
static DuinoCubeSystem sys;

void DuinoCubeRPC::begin() {
  writeCommand(RPC_CMD_NONE);

  // Reset the RPC server.
  digitalWrite(sys.s_ss_pin, LOW);
  for (uint8_t i = 0; i < NUM_RESET_CYCLES; ++i)
    SPI.transfer(OP_RESET);
  digitalWrite(sys.s_ss_pin, HIGH);
}

uint16_t DuinoCubeRPC::hello(uint16_t buf_addr) {
  RPC_HelloArgs args;
  args.in.buf_addr = buf_addr;
  uint16_t status = exec(RPC_CMD_HELLO, &args.in, sizeof(args.in), NULL, 0);

  return status;
}

uint16_t DuinoCubeRPC::invert(uint16_t buf_addr, uint16_t size) {
  RPC_InvertArgs args;
  args.in.buf_addr = buf_addr;
  args.in.size     = size;
  uint16_t status = exec(RPC_CMD_INVERT, &args.in, sizeof(args.in), NULL, 0);

  return status;
}

void DuinoCubeRPC::writeCommand(uint8_t value) {
  digitalWrite(sys.s_ss_pin, LOW);
  SPI.transfer(OP_WRITE_COMMAND);
  SPI.transfer(value);
  digitalWrite(sys.s_ss_pin, HIGH);
}

uint8_t DuinoCubeRPC::readServerStatus() {
  digitalWrite(sys.s_ss_pin, LOW);
  SPI.transfer(OP_READ_STATUS);
  uint8_t result = SPI.transfer(0);
  digitalWrite(sys.s_ss_pin, HIGH);

  return result;
}

void DuinoCubeRPC::waitForServerStatus(uint8_t status) {
  // Must read the same status twice in a row to avoid race conditions.
  while (readServerStatus() != status || readServerStatus() != status);
}

uint16_t DuinoCubeRPC::exec(uint8_t command,
                            const void* in_args, uint8_t in_size,
                            void* out_args, uint8_t out_size) {
  // Wait for the server to be ready.
  // TODO: add a timeout mechanism or fail immediately if not ready?
  waitForServerStatus(RPC_CMD_NONE);

  // Write the command input args to memory.
  if (in_args && in_size > 0)
    sys.writeSharedRAM(INPUT_ARG_ADDR, in_args, in_size);

  // Issue the command and wait for acknowledgment.
  // TODO: figure out why writing it only once will sometimes cause a hang.
  writeCommand(command);
  writeCommand(command);
  waitForServerStatus(command);

  // Clear the command status register.
  writeCommand(RPC_CMD_NONE);
  writeCommand(RPC_CMD_NONE);

  // Now wait for the RPC to complete.
  waitForServerStatus(RPC_CMD_NONE);

  if (out_args && out_size > 0)
    sys.readSharedRAM(OUTPUT_ARG_ADDR, out_args, out_size);

  // TODO: implement status codes.
  return 0;
}

