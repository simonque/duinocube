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

// DuinoCube coprocessor firmware.

#include <stdio.h>

#include <avr/io.h>

#include "DuinoCube_defs.h"
#include "DuinoCube_rpc.h"

#include "defines.h"
#include "spi.h"
#include "uart.h"

static uint8_t read_client_command() {
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_READ_COMMAND);
  uint8_t value = spi_tx(0);
  spi_set_ss(DEV_SELECT_NONE);

  return value;
}

static void write_server_status(uint8_t value) {
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_WRITE_STATUS);
  spi_tx(value);
  spi_set_ss(DEV_SELECT_NONE);
}

#define RAM_WRITE    2
#define RAM_READ     3

#define RAM_ST_WRITE 1
#define RAM_ST_READ  5

static void ram_write(uint16_t addr, const void* data, uint16_t len) {
  const char* buf = (const char*)data;
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_ACCESS_RAM);
  SPCR &= ~(1 << DORD);

  spi_tx(RAM_WRITE);
  spi_tx(addr >> 8);
  spi_tx((uint8_t) addr);
  for (uint16_t i = 0; i < len; ++i)
    spi_tx(buf[i]);

  spi_set_ss(DEV_SELECT_NONE);
  SPCR |= (1 << DORD);
}

static void ram_read(uint16_t addr, void* data, uint16_t len) {
  char* buf = (char*)data;
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_ACCESS_RAM);

  SPCR &= ~(1 << DORD);
  spi_tx(RAM_READ);
  spi_tx(addr >> 8);
  spi_tx((uint8_t) addr);
  for (uint16_t i = 0; i < len; ++i)
    buf[i] = spi_tx(0);

  spi_set_ss(DEV_SELECT_NONE);
  SPCR |= (1 << DORD);
}

static void rpc_hello(RPC_HelloArgs* args) {
  // TODO: put this into progmem.
  const char str[20] = "Hello world.";
  ram_write(args->in.buf_addr, str, sizeof(str));
}

static void rpc_invert(RPC_InvertArgs* args) {
  char buf[256];
  ram_read(args->in.buf_addr, buf, args->in.size);
  for (uint16_t offset = 0; offset < args->in.size; ++offset)
    buf[offset] = ~buf[offset];
  ram_write(args->in.buf_addr, buf, args->in.size);
}

static void rpc_exec(uint8_t command) {
  switch(command) {
  case RPC_CMD_HELLO: {
    RPC_HelloArgs hello;
    ram_read(INPUT_ARG_ADDR, &hello.in, sizeof(hello.in));
    rpc_hello(&hello);
    // No outputs to write.
    break;
  }
  case RPC_CMD_INVERT: {
    RPC_InvertArgs invert;
    ram_read(INPUT_ARG_ADDR, &invert.in, sizeof(invert.in));
    rpc_invert(&invert);
    // No outputs to write.
    break;
  }
  default:
    // Handle unrecognized command.
    break;
  }
}

int main() {
  uart_init();
  spi_init();

#if DEBUG
  printf("\n\nSystem initialized.\n");
#endif

  while (true) {
    // Set RPC status to ready.
    write_server_status(RPC_CMD_NONE);

    // Read in RPC client status.
    // Read it again to make sure the value is steady.
    while (read_client_command() == RPC_CMD_NONE ||
           read_client_command() == RPC_CMD_NONE);

    // Read the command and update the status register to indicate that the
    // command was received.
    uint8_t command = read_client_command();
    write_server_status(command);
    write_server_status(command);
    write_server_status(command);

#ifdef DEBUG
    printf("Received command code: 0x%02x\n", command);
#endif

    // Wait for MCU to clear the command register.
    while(read_client_command() != RPC_CMD_NONE ||
          read_client_command() != RPC_CMD_NONE);

#ifdef DEBUG
    printf("Executing command.\n");
#endif
    rpc_exec(command);

    // Finish the RPC operation.
    write_server_status(RPC_CMD_NONE);
  }

  return 0;
}
