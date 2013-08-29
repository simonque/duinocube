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

// DuinoCube coprocessor SPI functions.

#include <avr/io.h>

#include "defines.h"
#include "spi.h"

#define SS_MASK     0x07    // The SPI device select is three bits wide.

void spi_init(void) {
  DDRB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) |  // Enable select.
          (1 << PORTB3) |    // Enable MOSI.
          (1 << PORTB5);     // Enable SCK.
  SPCR = (1 << SPE) |     // Enable SPI.
         (1 << MSTR) |    // SPI master mode.
         (1 << DORD);     // LSB first.
                          // TODO: convert to MSB first.

  // Clock  speed of CPU clock / 2. (Max possible)
  SPCR |= (0 << SPR1) | (0 << SPR0);
  SPSR |= (1 << SPI2X);

  spi_set_ss(DEV_SELECT_NONE);
}

uint8_t spi_tx(uint8_t value) {
  SPDR = value;
  while(!(SPSR & (1 << SPIF)));
  return SPDR;
}

void spi_set_ss(uint8_t ss_value) {
  PORTB = (PORTB & ~(SS_MASK)) | (ss_value & SS_MASK);
}
