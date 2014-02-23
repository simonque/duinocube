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

// DuinoCube System Shield shared memory allocation library.

#include "mem.h"

#include <stdio.h>

#include "rpc.h"
#include "rpc_mem.h"

static DuinoCubeRPC rpc;    // For RPC layer access.

void DuinoCubeMemory::stat(uint16_t* total_free_size,
                           uint16_t* largest_free_size) {
  RPC_MemStatArgs args;

  rpc.exec(RPC_CMD_MEM_STAT, NULL, 0, &args.out, sizeof(args.out));

  if (total_free_size)
    *total_free_size = args.out.total_free_size;
  if (largest_free_size)
    *largest_free_size = args.out.largest_free_size;
}

uint16_t DuinoCubeMemory::alloc(uint16_t size) {
  RPC_MemAllocArgs args;
  args.in.size = size;

  rpc.exec(RPC_CMD_MEM_ALLOC,
           &args.in, sizeof(args.in),
           &args.out, sizeof(args.out));

  return args.out.addr;
}

void DuinoCubeMemory::free(uint16_t addr) {
  RPC_MemFreeArgs args;
  args.in.addr = addr;

  rpc.exec(RPC_CMD_MEM_FREE, &args.in, sizeof(args.in), NULL, 0);
}
