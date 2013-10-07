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

// DuinoCube remote procedure call functions for memory allocation.

#include "DuinoCube_mem.h"
#include "DuinoCube_rpc_mem.h"

#include "shmem.h"

#include "rpc_mem.h"

void rpc_mem_stat() {
  RPC_MemStatArgs args;
  shmem_stat(&args.out.total_free_size, &args.out.largest_free_size);
  shmem_write(OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
}

void rpc_mem_alloc() {
  RPC_MemAllocArgs args;
  shmem_read(INPUT_ARG_ADDR, &args.in, sizeof(args.in));

  args.out.addr = shmem_alloc(args.in.size);

  shmem_write(OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
}

void rpc_mem_free() {
  RPC_MemFreeArgs args;
  shmem_read(INPUT_ARG_ADDR, &args.in, sizeof(args.in));

  shmem_free(args.in.addr);
}
