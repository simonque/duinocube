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

// DuinoCube Remote Procedure Call (RPC) definitions: shared memory allcoation
// operations.

#ifndef __DUINOCUBE_RPC_MEM_H__
#define __DUINOCUBE_RPC_MEM_H__

#include <stdint.h>

// Note: all fields are to be at least uint16_t for alignment compatibility
// between 8-bit and non-8-bit systems (e.g. ARM).

struct RPC_MemStatArgs {
  // No inputs.
  struct {
    uint16_t total_free_size;     // Number of bytes free.
    uint16_t largest_free_size;   // Size in bytes of largest continguous
                                  // unallocated region.
  } out;
};

struct RPC_MemAllocArgs {
  struct {
    uint16_t size;                // Number of bytes to allocate.
  } in;
  struct {
    uint16_t addr;                // Address of allocated memory.
  } out;
};

struct RPC_MemFreeArgs {
  struct {
    uint16_t addr;                // Address of allocated memory to free.
  } in;
  // No outputs.
};

#endif  // __DUINOCUBE_RPC_MEM_H__
