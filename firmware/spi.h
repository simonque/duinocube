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

// DuinoCube coprocessor SPI header.

#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

// Bit order enum defs.
enum {
  SPI_MSB_FIRST,
  SPI_LSB_FIRST,
};

// Initializes the SPI system.
void spi_init();

// Set bit order of SPI system.
void spi_set_bit_order(uint8_t order);

// Send a byte |value| over SPI.  Also returns the value read from SPI bus in
// the same operation.
uint8_t spi_tx(uint8_t value);

// Functions to set and clear the SPI device select pins.
void spi_set_ss(uint8_t bit);
void spi_clear_ss(uint8_t bit);

#endif  // __SPI_H__
