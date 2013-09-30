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

// DuinoCube remote procedure call definitions for memory allocation.

#ifndef __RPC_MEM_H__
#define __RPC_MEM_H__

#include <stdint.h>

#include "DuinoCube_rpc_mem.h"

void rpc_mem_stat();
void rpc_mem_alloc();
void rpc_mem_free();

#endif  // __RPC_MEM_H__
