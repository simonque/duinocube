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

// DuinoCube coprocessor firmware.

#include <avr/io.h>

#include "DuinoCube_defs.h"
#include "DuinoCube_rpc.h"

#include "defines.h"
#include "file.h"
#include "printf.h"
#include "rpc.h"
#include "shmem.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "usb.h"

const char main_str0[] PROGMEM = "\n\nSystem initialized.\n";

int main() {
  // Initialize microcontroller peripherals.
  uart_init();
  spi_init();
  timer_init();

  // Initialize firmware components.
  shmem_init();
  file_init();
  usb_init();

  rpc_init();

#if DEBUG
  printf_P(main_str0);
#endif

  // Start RPC server loop.
  rpc_server_loop();

  return 0;
}
