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

// DuinoCube remote procedure call functions for file system.

#include <stdio.h>

#include "DuinoCube_rpc.h"

#include "file.h"
#include "shmem.h"

#include "rpc_file.h"

// Macro that returns the smaller of the two values.
#define MIN(a,b) ((a)<(b)?(a):(b))

// Size of intermediary buffer used for data transfers between files and shared
// memory.
#define FILE_BUFFER_SIZE        256

void rpc_file_open() {
  RPC_FileOpenArgs args;
  shmem_read(INPUT_ARG_ADDR, &args.in, sizeof(args.in));

  char filename_buf[STRING_BUF_SIZE];
  shmem_read(args.in.filename_addr, filename_buf, STRING_BUF_SIZE);

  args.out.handle = (uint16_t) file_open(filename_buf, args.in.mode);

  shmem_write(OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
}

void rpc_file_close() {
  RPC_FileCloseArgs args;
  shmem_read(INPUT_ARG_ADDR, &args.in, sizeof(args.in));

  file_close(args.in.handle);
}

void rpc_file_read() {
  RPC_FileReadArgs args;
  shmem_read(INPUT_ARG_ADDR, &args.in, sizeof(args.in));

  // Since file access can only take place between local memory space and the
  // file, use a buffer as an intermediate between the file and shared memory.
  uint8_t buffer[FILE_BUFFER_SIZE];
  uint16_t total_size_read;
  for (total_size_read = 0;
       total_size_read < args.in.size;
       total_size_read += FILE_BUFFER_SIZE) {
    // Determine the amount of data to read in the next read operation.
    uint16_t size_to_read =
        MIN(args.in.size - total_size_read, FILE_BUFFER_SIZE);
    // Read it and store the data into shared memory.
    uint16_t size_just_read = file_read(args.in.handle, buffer, size_to_read);
    shmem_write(args.in.dst_addr + total_size_read, buffer, size_just_read);

    // If there wasn't enough data remaining in the file to read |size_to_read|
    // bytes, it means that the actual file data is less than |args.in.size|.
    // Break out of the loop.
    if (size_to_read != size_just_read) {
      total_size_read += size_just_read;
      break;
    }
  }
  args.out.size_read = total_size_read;

  shmem_write(OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
}

void rpc_file_write() {
  RPC_FileWriteArgs args;
  shmem_read(INPUT_ARG_ADDR, &args.in, sizeof(args.in));

  // Since file access can only take place between local memory space and the
  // file, use a buffer as an intermediate between the file and shared memory.
  uint8_t buffer[FILE_BUFFER_SIZE];
  uint16_t total_size_written;
  for (total_size_written = 0;
       total_size_written < args.in.size;
       total_size_written += FILE_BUFFER_SIZE) {
    // Determine the amount of data to write in the next write operation.
    uint16_t size_to_write =
        MIN(args.in.size - total_size_written, FILE_BUFFER_SIZE);

    // Get the next chunk of data from shared memory and write it to file.
    shmem_read(args.in.src_addr + total_size_written, buffer, size_to_write);
    uint16_t size_just_written =
        file_read(args.in.handle, buffer, size_to_write);

    // If writing |size_to_write| bytes to file failed, the file system may have
    // run out of space, so quit without having written |args.in.size| bytes.
    if (size_to_write != size_just_written) {
      total_size_written += size_just_written;
      break;
    }
  }
  args.out.size_written = total_size_written;

  shmem_write(OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
}
