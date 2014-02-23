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

// DuinoCube File I/O library for Arduino.

#include "file.h"

#include <stdio.h>
#include <string.h>

#include "core.h"
#include "rpc.h"
#include "rpc_file.h"
#include "system.h"

namespace DuinoCube {

// Local handles to other DuinoCube classes.
static Core core;
static System sys;
static RPC rpc;

uint16_t File::open(const char* filename, uint16_t mode) {
  RPC_FileOpenArgs args;
  args.in.filename_addr = STRING_BUF_ADDR;
  args.in.mode          = mode;

  // Copy the name string to shared memory (including null terminator).
  sys.writeSharedRAM(STRING_BUF_ADDR, filename, strlen(filename) + 1);

  rpc.exec(RPC_CMD_FILE_OPEN,
           &args.in, sizeof(args.in),
           &args.out, sizeof(args.out));

  return args.out.handle;
}

void File::close(uint16_t handle) {
  RPC_FileCloseArgs args;
  args.in.handle = handle;

  rpc.exec(RPC_CMD_FILE_CLOSE, &args.in, sizeof(args.in), NULL, 0);
}

uint16_t File::read(uint16_t handle, uint16_t dst_addr, uint16_t size) {
  RPC_FileReadArgs args;
  args.in.handle     = handle;
  args.in.dst_addr   = dst_addr;
  args.in.size       = size;

  rpc.exec(RPC_CMD_FILE_READ,
           &args.in, sizeof(args.in),
           &args.out, sizeof(args.out));

  return args.out.size_read;
}

uint16_t File::write(uint16_t handle, uint16_t src_addr, uint16_t size) {
  RPC_FileWriteArgs args;
  args.in.handle     = handle;
  args.in.src_addr   = src_addr;
  args.in.size       = size;

  rpc.exec(RPC_CMD_FILE_WRITE,
           &args.in, sizeof(args.in),
           &args.out, sizeof(args.out));

  return args.out.size_written;
}

uint16_t File::readToCore(uint16_t handle, uint16_t dst_addr, uint16_t size) {
  // The Core memory space is at 0x8000, from the coprocessor's point of view.
  // TODO: get rid of magic number.
  // TODO: consider a better memory mapping scheme.
  uint16_t size_read = read(handle, dst_addr | 0x8000, size);
  return size_read;
}

uint32_t File::size(uint16_t handle) {
  RPC_FileSizeArgs args;
  args.in.handle = handle;

  rpc.exec(RPC_CMD_FILE_SIZE,
           &args.in, sizeof(args.in),
           &args.out, sizeof(args.out));

  return args.out.size;
}

void File::seek(uint16_t handle, uint32_t offset) {
  RPC_FileSeekArgs args;
  args.in.handle = handle;
  args.in.offset = offset;

  rpc.exec(RPC_CMD_FILE_SEEK, &args.in, sizeof(args.in), NULL, 0);
}

}  // namespace DuinoCube
