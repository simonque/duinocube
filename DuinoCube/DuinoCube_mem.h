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

// DuinoCube System Shield shared memory allocation library.

#ifndef __DUINOCUBE_MEM_H__
#define __DUINOCUBE_MEM_H__

#include <stdint.h>

#define SHARED_MEMORY_SIZE   0x8000   // System contains 32KB of shared memory.

// Addresses of RPC input and output args in shared memory.
#define RPC_COMMAND_ADDR     0x0000
#define RPC_INPUT_ARG_ADDR   (RPC_COMMAND_ADDR + 0x10)
#define RPC_OUTPUT_ARG_ADDR  0x0000

// The default buffer address and size for passing shared memory strings.
#define STRING_BUF_ADDR      0x0100
#define STRING_BUF_SIZE         256

// Heap starts after the statically allocated regions.
#define SHARED_MEMORY_HEAP_START     0x200
#define SHARED_MEMORY_HEAP_SIZE      \
    (SHARED_MEMORY_SIZE - SHARED_MEMORY_HEAP_START)
#define SHARED_MEMORY_BLOCK_SIZE     256  // Size of heap alloc chunk.

class DuinoCubeMemory {
 public:
  // Gets stats about shared memory allocation usage.
  static void stat(uint16_t* free_size, uint16_t* largest_size);

  // Alloc and free shared memory.
  static uint16_t alloc(uint16_t size);
  static void free(uint16_t addr);
};

#endif  // __DUINOCUBE_MEM_H__
