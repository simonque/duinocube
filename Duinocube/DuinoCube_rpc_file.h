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

// DuinoCube Remote Procedure Call (RPC) definitions: file system operations.

#ifndef __DUINOCUBE_RPC_FILE_H__
#define __DUINOCUBE_RPC_FILE_H__

#include <stdint.h>

// Note: all fields are to be at least uint16_t for alignment compatibility
// between 8-bit and non-8-bit systems (e.g. ARM).

struct RPC_FileOpenArgs {
  struct {
    uint16_t filename_addr;     // Address of filename string in shared memory.
    uint16_t mode;              // File open mode.
  } in;
  struct {
    uint16_t handle;            // Handle to opened file.
  } out;
};

struct RPC_FileCloseArgs {
  struct {
    uint16_t handle;            // File handle to close.
  } in;
  // No outputs.
};

struct RPC_FileReadArgs {
  struct {
    uint16_t handle;            // File handle.
    uint16_t dst_addr;          // Address of shared memory to read to.
    uint16_t size;              // Number of bytes to read.
  } in;
  struct {
    uint16_t size_read;         // Actual number of bytes read.
  } out;
};

struct RPC_FileWriteArgs {
  struct {
    uint16_t handle;            // File handle.
    uint16_t src_addr;          // Address of shared memory to write from.
    uint16_t size;              // Number of bytes to write.
  } in;
  struct {
    uint16_t size_written;      // Actual number of bytes written.
  } out;
};

struct RPC_FileSizeArgs {
  struct {
    uint16_t handle;            // File handle.
  } in;
  struct {
    uint32_t size;              // Number of bytes in file.
  } out;
};

struct RPC_FileSeekArgs {
  struct {
    uint16_t handle;            // File handle.
    uint32_t offset;            // Offset to seek.
  } in;
};

#endif  // __DUINOCUBE_RPC_FILE_H__
