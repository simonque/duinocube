// Copyright (C) 2014 Simon Que
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

// Font system.

#include "font.h"

#include <stdint.h>
#include <string.h>

#include "font8x8.h"
#include "utils.h"

// Represented as one bit per pixel.
#define RAW_FONT_CHAR_SIZE       (FONT_CHAR_SIZE / 8)

void font_load_bitmap(char ch, void* data) {
  uint8_t* buf = reinterpret_cast<uint8_t*>(data);

  // Load the raw font data.
  uint8_t raw_font_data[RAW_FONT_CHAR_SIZE];
  memcpy_P(raw_font_data, kFont8x8_basic + RAW_FONT_CHAR_SIZE * ch,
           RAW_FONT_CHAR_SIZE);

  for (uint8_t row = 0; row < FONT_CHAR_HEIGHT; ++row) {
    uint8_t line_data = raw_font_data[row];
    for (uint8_t col = 0; col < FONT_CHAR_WIDTH; ++col) {
      *buf++ = (line_data & 1);
      line_data >>= 1;
    }
  }
}
