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

// DuinoCube Remote Procedure Call (RPC) definitions.

#ifndef __DUINOCUBE_RPC_H___
#define __DUINOCUBE_RPC_H__

#include <stdint.h>

#include "DuinoCube_mem.h"

// RPC command codes.  These should be consistent with definitions in the
// chronocube repo, for the purposes of RAM bus arbitration.
enum {

  RPC_CMD_NONE = 0x00,          // The NOP value for the command register.

  // Test commands.
  RPC_CMD_HELLO = 0x10,         // Test function that writes "hello world".
  RPC_CMD_INVERT,               // Test function that inverts data.

  // File I/O commands.
  RPC_CMD_FILE_INIT = 0x20,     // Initialize the file system.
  RPC_CMD_FILE_INFO,            // Gets the state of the file system.
  RPC_CMD_FILE_OPEN,            // Open a file handle.
  RPC_CMD_FILE_CLOSE,           // Close a file handle.
  RPC_CMD_FILE_READ,            // Read data from a file handle.
  RPC_CMD_FILE_WRITE,           // Write data to a file handle.
  RPC_CMD_FILE_STATS,           // Get info about an open file and its handle.
  RPC_CMD_FILE_SEEK,            // Move file handle pointer.

  // Shared memory allocation commands.
  RPC_CMD_MEM_INIT = 0x30,      // Initialize shared memory heap.
  RPC_CMD_MEM_STAT,             // Get stats about shared memory heap.
  RPC_CMD_MEM_ALLOC,            // Allocate memory from heap.
  RPC_CMD_MEM_FREE,             // Free memory allocated from heap.

  // USB and Joystick commands.
  RPC_CMD_USB_INIT = 0x40,      // Initialize USB host.
  RPC_CMD_USB_STATUS,           // Get USB device status.
  RPC_CMD_USB_READ_JOYSTICK,    // Get USB joystick and button state.

};  // enum

// RPC argument structures.

// For RPC_CMD_HELLO.
typedef struct {
  struct {
    uint16_t buf_addr;          // Shared memory address of buffer.
  } in;
  // No outputs.
} RPC_HelloArgs;

// For RPC_CMD_INVERT.
typedef struct {
  struct {
    uint16_t buf_addr;          // Shared memory address of buffer.
    uint16_t size;              // Length in bytes of data to invert.
  } in;
  // No outputs.
} RPC_InvertArgs;

class DuinoCubeRPC {
 public:
  static void begin();

  // RPC test functions.
  static uint16_t hello(uint16_t buf_addr);
  static uint16_t invert(uint16_t buf_addr, uint16_t size);

  // Writes a byte to the RPC status register.
  static void writeCommand(uint8_t value);
  // Writes a byte to the coprocessor status register.
  static uint8_t readServerStatus();

  // Waits for the RPC Server status to become |status|.
  static void waitForServerStatus(uint8_t status);

  // Executes an RPC function.
  static uint16_t exec(uint8_t command,
                       const void* in_args, uint8_t in_size,
                       void* out_args, uint8_t out_size);
};

#endif  // __DUINOCUBE_RPC_H__
