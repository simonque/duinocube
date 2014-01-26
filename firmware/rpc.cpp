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

// DuinoCube remote procedure call functions.

#include <string.h>

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
  uint8_t command_pin_value = (PINB >> RPC_COMMAND_BIT) & 1;
  if (command_pin_value == RPC_CLIENT_NO_COMMAND)
    return RPC_CMD_NONE;
  uint8_t command;
  shmem_read(RPC_COMMAND_ADDR, &command, sizeof(command));
  return command;
}

// Write an RPC server status code for the RPC client to read.
static void set_server_status(uint8_t status) {
  switch (status) {
  case RPC_SERVER_IDLE:
    PORTB &= ~(1 << RPC_STATUS_BIT);
    break;
  case RPC_SERVER_BUSY:
    PORTB |= (1 << RPC_STATUS_BIT);
    break;
  }
}

const char rpc_hello_str0[] PROGMEM = "Hello world.";

// Test function that writes a string to a buffer.
static void rpc_hello(RPC_HelloArgs* args) {
  char str[20];
  memset(str, 0, sizeof(str));
  for (size_t i = 0; i < sizeof(str) && pgm_read_byte(rpc_hello_str0[i]); ++i)
    str[i] = pgm_read_byte(rpc_hello_str0 + i);
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
    shmem_read(RPC_INPUT_ARG_ADDR, &hello.in, sizeof(hello.in));
    rpc_hello(&hello);
    // No outputs to write.
    break;
  }
  case RPC_CMD_INVERT: {
    RPC_InvertArgs invert;
    shmem_read(RPC_INPUT_ARG_ADDR, &invert.in, sizeof(invert.in));
    rpc_invert(&invert);
    // No outputs to write.
    break;
  }
  case RPC_CMD_READ_CORE_ID: {
    RPC_ReadCoreIDArgs args;
    // TODO: add a macro to make it more clear that |SHARED_MEMORY_SIZE| is
    // actually address 0 of the Core address space.
    shmem_read(SHARED_MEMORY_SIZE, &args.out.id, sizeof(args.out.id));
    shmem_write(RPC_OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
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
  case RPC_CMD_FILE_SIZE:
    rpc_file_size();
    break;
  case RPC_CMD_FILE_SEEK:
    rpc_file_seek();
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
  DDRB |= (1 << RPC_STATUS_BIT);     // The RPC server status is an output.
  DDRB &= ~(1 << RPC_COMMAND_BIT);   // The RPC command status is an input.

  // Set server status to ready.
  set_server_status(RPC_SERVER_IDLE);
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
    set_server_status(RPC_SERVER_IDLE);

    // Read in RPC client status.
    do {
      rpc_idle();
    } while (read_client_command() == RPC_CMD_NONE);

    // Read the command and update the status register to indicate that the
    // command was received.
    uint8_t command = read_client_command();
    set_server_status(RPC_SERVER_BUSY);

#ifdef DEBUG
    printf_P(rpc_server_loop_str0, command);
#endif

    // Wait for MCU to clear the command register.
    while(read_client_command() != RPC_CMD_NONE);

#ifdef DEBUG
    printf_P(rpc_server_loop_str1);
#endif
    rpc_exec(command);
#ifdef DEBUG
    printf_P(rpc_server_loop_str2);
#endif

    // Finish the RPC operation.
    set_server_status(RPC_SERVER_IDLE);
  }
}
