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

// DuinoCube shared memory functions.

#ifndef __SHMEM_H__
#define __SHMEM_H__

#include <stdint.h>

// Set up shared memory.
void shmem_init();

// Shared memory access functions.
void shmem_read(uint16_t addr, void* data, uint16_t len);
void shmem_write(uint16_t addr, const void* data, uint16_t len);

#endif  // __SHMEM_H__
