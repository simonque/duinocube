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

// NOTE: These should match the verilog definitions in the chronocube repo.

// RPC Client (Arduino) statuses.
#define MCU_RPC_NONE              0
#define MCU_RPC_ISSUED            1
#define MCU_RPC_WAITING           2

// RPC Server (Coprocessor) statuses.
#define COP_RPC_POWER_ON          0
#define COP_RPC_READY             1
#define COP_RPC_RECEIVED          2
#define COP_RPC_DONE              3

// RPC command codes.
enum {

  // Test commands.
  RPC_CMD_HELLO = 0x00,         // Test function that writes "hello world".
  RPC_CMD_INVERT,               // Test function that inverts data.

  // File I/O commands.
  RPC_CMD_FILE_INIT = 0x10,     // Initialize the file system.
  RPC_CMD_FILE_INFO,            // Gets the state of the file system.
  RPC_CMD_FILE_OPEN,            // Open a file handle.
  RPC_CMD_FILE_CLOSE,           // Close a file handle.
  RPC_CMD_FILE_READ,            // Read data from a file handle.
  RPC_CMD_FILE_WRITE,           // Write data to a file handle.
  RPC_CMD_FILE_STATS,           // Get info about an open file and its handle.
  RPC_CMD_FILE_SEEK,            // Move file handle pointer.

};  // enum

typedef struct {
  struct {
    uint16_t buf_addr;          // Shared memory address of buffer.
  } in;
  struct {
    // No outputs.
  } out;
} RPC_HelloArgs;

typedef struct {
  struct {
    uint16_t buf_addr;          // Shared memory address of buffer.
    uint16_t size;              // Length in bytes of data to invert.
  } in;
  struct {
    // No outputs.
  } out;
} RPC_InvertArgs;

#endif  // __DUINOCUBE_RPC_H__
