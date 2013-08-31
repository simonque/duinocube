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

#include "defines.h"
#include "spi.h"

#include "shmem.h"

#define RAM_WRITE    2
#define RAM_READ     3

void shmem_init() {
  // Nothing to do since the shared memory should have been initialized by the
  // main microcontroller.  Leave this stub so things can be added in the
  // future.
}

void shmem_read(uint16_t addr, void* data, uint16_t len) {
  char* buf = (char*)data;
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_ACCESS_RAM);

  spi_tx(RAM_READ);
  spi_tx(addr >> 8);
  spi_tx((uint8_t) addr);
  for (uint16_t i = 0; i < len; ++i)
    buf[i] = spi_tx(0);

  spi_set_ss(DEV_SELECT_NONE);
}

void shmem_write(uint16_t addr, const void* data, uint16_t len) {
  const char* buf = (const char*)data;
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_ACCESS_RAM);

  spi_tx(RAM_WRITE);
  spi_tx(addr >> 8);
  spi_tx((uint8_t) addr);
  for (uint16_t i = 0; i < len; ++i)
    spi_tx(buf[i]);

  spi_set_ss(DEV_SELECT_NONE);
}
