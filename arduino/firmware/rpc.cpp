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

// DuinoCube remote procedure call functions.

#include "DuinoCube_rpc.h"
#include "DuinoCube_rpc_file.h"
#include "DuinoCube_rpc_mem.h"
#include "DuinoCube_rpc_usb.h"

#include "defines.h"
#include "printf.h"
#include "rpc_file.h"
#include "rpc_mem.h"
#include "rpc_usb.h"
#include "shmem.h"
#include "spi.h"
#include "usb.h"

#include "rpc.h"

// Reads the command code issued by the RPC client.
static uint8_t read_client_command() {
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_READ_COMMAND);
  uint8_t value = spi_tx(0);
  spi_set_ss(DEV_SELECT_NONE);

  return value;
}

// Write an RPC server status code for the RPC client to read.
static void write_server_status(uint8_t value) {
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_WRITE_STATUS);
  spi_tx(value);
  spi_set_ss(DEV_SELECT_NONE);
}

const char rpc_hello_str0[] PROGMEM = "Hello world.";

// Test function that writes a string to a buffer.
static void rpc_hello(RPC_HelloArgs* args) {
  char str[20];
  for (size_t i = 0; i < sizeof(str); ++i)
    str[i] = pgm_read_byte(rpc_hello_str0);
  shmem_write(args->in.buf_addr, str, sizeof(str));
}

// Test function that inverts a buffer.
static void rpc_invert(RPC_InvertArgs* args) {
  char buf[256];
  shmem_read(args->in.buf_addr, buf, args->in.size);
  for (uint16_t offset = 0; offset < args->in.size; ++offset)
    buf[offset] = ~buf[offset];
  shmem_write(args->in.buf_addr, buf, args->in.size);
}

static void rpc_exec(uint8_t command) {
  switch(command) {
  case RPC_CMD_HELLO: {
    RPC_HelloArgs hello;
    shmem_read(INPUT_ARG_ADDR, &hello.in, sizeof(hello.in));
    rpc_hello(&hello);
    // No outputs to write.
    break;
  }
  case RPC_CMD_INVERT: {
    RPC_InvertArgs invert;
    shmem_read(INPUT_ARG_ADDR, &invert.in, sizeof(invert.in));
    rpc_invert(&invert);
    // No outputs to write.
    break;
  }
  case RPC_CMD_FILE_OPEN:
    rpc_file_open();
    break;
  case RPC_CMD_FILE_CLOSE:
    rpc_file_close();
    break;
  case RPC_CMD_FILE_READ:
    rpc_file_read();
    break;
  case RPC_CMD_FILE_WRITE:
    rpc_file_write();
    break;
  case RPC_CMD_MEM_STAT:
    rpc_mem_stat();
    break;
  case RPC_CMD_MEM_ALLOC:
    rpc_mem_alloc();
    break;
  case RPC_CMD_MEM_FREE:
    rpc_mem_free();
    break;

  case RPC_CMD_USB_READ_JOYSTICK:
    rpc_usb_read_joystick();
    break;

  default:
    // Handle unrecognized command.
    break;
  }
}

void rpc_init() {
  // Nothing to initialize right now.  Leave this stub here so things can be
  // added in the future.
}

// Function to run when not executing any RPC commands.
static void rpc_idle() {
  // Poll USB.
  // TODO: Separate this from RPC.
  usb_update();
}

const char rpc_server_loop_str0[] PROGMEM = "Received command code: 0x%02x\n";
const char rpc_server_loop_str1[] PROGMEM = "Executing command.\n";
const char rpc_server_loop_str2[] PROGMEM = "Done executing command.\n\n";

void rpc_server_loop() {
  while (true) {
    // Set RPC status to ready.
    write_server_status(RPC_CMD_NONE);

    // Read in RPC client status.
    // Read it again to make sure the value is steady.
    do {
      rpc_idle();
    } while (read_client_command() == RPC_CMD_NONE ||
             read_client_command() == RPC_CMD_NONE);

    // Read the command and update the status register to indicate that the
    // command was received.
    uint8_t command = read_client_command();
    write_server_status(command);
    write_server_status(command);
    write_server_status(command);

#ifdef DEBUG
    printf_P(rpc_server_loop_str0, command);
#endif

    // Wait for MCU to clear the command register.
    while(read_client_command() != RPC_CMD_NONE ||
          read_client_command() != RPC_CMD_NONE);

#ifdef DEBUG
    printf_P(rpc_server_loop_str1);
#endif
    rpc_exec(command);
#ifdef DEBUG
    printf_P(rpc_server_loop_str2);
#endif

    // Finish the RPC operation.
    write_server_status(RPC_CMD_NONE);
  }
}
