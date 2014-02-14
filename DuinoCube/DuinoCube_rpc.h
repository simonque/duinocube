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

// DuinoCube Remote Procedure Call (RPC) definitions.

#ifndef __DUINOCUBE_RPC_H___
#define __DUINOCUBE_RPC_H__

#include <stdint.h>

#include "DuinoCube_mem.h"

// Server and client status values.
#define RPC_SERVER_BUSY            0    // Server is executing a command.
#define RPC_SERVER_IDLE            1    // Server is idle, ready for commands.
#define RPC_CLIENT_COMMAND         0    // Client has issued a command.
#define RPC_CLIENT_NO_COMMAND      1    // Client has issued no command.

// RPC command codes.  These should be consistent with definitions in the
// chronocube repo, for the purposes of RAM bus arbitration.
enum {

  RPC_CMD_NONE = 0x00,              // The NOP RPC command value.

  // Test commands.
  RPC_CMD_HELLO = 0x10,             // Test function that writes "hello world".
  RPC_CMD_INVERT,                   // Test function that inverts data.
  RPC_CMD_READ_CORE_ID,             // Reads and returns the Core ID.

  // File I/O commands.
  RPC_CMD_FILE_INFO = 0x20,         // Gets the state of the file system.
  RPC_CMD_FILE_OPEN,                // Open a file handle.
  RPC_CMD_FILE_CLOSE,               // Close a file handle.
  RPC_CMD_FILE_READ,                // Read data from a file handle.
  RPC_CMD_FILE_WRITE,               // Write data to a file handle.
  RPC_CMD_FILE_SIZE,                // Get the file size in bytes.
  RPC_CMD_FILE_SEEK,                // Move file handle pointer.

  // Shared memory allocation commands.
  RPC_CMD_MEM_STAT = 0x30,          // Get stats about shared memory heap.
  RPC_CMD_MEM_ALLOC,                // Allocate memory from heap.
  RPC_CMD_MEM_FREE,                 // Free memory allocated from heap.

  // USB and Joystick commands.
  RPC_CMD_USB_STATUS = 0x40,        // Get USB device status.
  RPC_CMD_USB_READ_JOYSTICK,        // Get USB joystick and button state.

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

// For RPC_CMD_READ_CORE_ID.
typedef struct {
  // No inputs.
  struct {
    uint16_t id;                // ID code that was read.
  } out;
} RPC_ReadCoreIDArgs;

class DuinoCubeRPC {
 public:
  static void begin();

  // Executes an RPC function.
  static uint16_t exec(uint8_t command,
                       const void* in_args, uint8_t in_size,
                       void* out_args, uint8_t out_size);

  // RPC test functions.
  static uint16_t hello(uint16_t buf_addr);
  static uint16_t invert(uint16_t buf_addr, uint16_t size);
  static uint16_t readCoreID();

 private:
  // Sets the client command status pin.
  static void setCommandStatus(uint8_t value);
  // Writes an RPC command code to shared memory.
  static void writeCommand(uint8_t command);
  // Reads the server status pin.
  static uint8_t readServerStatus();

  // Waits for the RPC Server status to become |status|.
  static void waitForServerStatus(uint8_t status);

};

#endif  // __DUINOCUBE_RPC_H__
