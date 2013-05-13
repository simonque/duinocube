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

// Prints strings using 8x8 tiles.

#include <stdint.h>
#include <stdio.h>

#include "cc_base.h"
#include "cc_core.h"
#include "cc_tile_layer.h"
#include "tile_registers.h"

#include "font_8x8.h"
#include "system.h"

int main(void)
{
  system_init();
  CC_Init();

  // Set up tile maps for two tile layers.
  const char* test_string = "Hello world.\n";
  // First map uses 16-bit tile map values.
  uint16_t buffer[32];
  memset(buffer, 0, sizeof(buffer));
  for (int i = 0; i < strlen(test_string); ++i)
    buffer[i] = test_string[i];
  CC_TileLayer_SetData(buffer, 0, 0, sizeof(buffer));
  // Second map uses 8-bit tile map values.
  CC_TileLayer_SetData(test_string, 1, 64, strlen(test_string));

  // Set up palette.
  uint32_t palette[2] = { 0x0000000, 0x00ffffff };
  CC_SetPaletteData(palette, 0, 0, sizeof(palette));

  // Set up font.
  uint16_t offset = 0;
  for (int i = 0; i < NUM_FONT_CHARS; ++i) {
    char c = i;
    for (int line = 0; line < NUM_FONT_CHAR_LINES; ++line) {
      uint8_t line_data = get_font_line(c, line);
      uint8_t line_buffer[8];
      for (int b = 0; b < sizeof(line_buffer) / sizeof(line_buffer[0]); ++b)
        line_buffer[b] = (line_data & (1 << b)) ? 1 : 0;
      CC_SetVRAMData(line_buffer, offset, sizeof(line_buffer));
      offset += sizeof(line_buffer);
    }
  }

  // Enable the tile layers.
  for (int i = 0; i < 2; ++i) {
    CC_TileLayer_SetRegister(i, TILE_CTRL0, (1 << TILE_LAYER_ENABLED) |
                        (1 << TILE_ENABLE_8x8) |
                        // For second layer, use 8-bit tile values.
                        (i << TILE_ENABLE_8_BIT) |
                        (1 << TILE_ENABLE_TRANSP) |
                        (1 << TILE_ENABLE_NOP));
    CC_TileLayer_SetRegister(i, TILE_NOP_VALUE, 0);
    CC_TileLayer_SetRegister(i, TILE_DATA_OFFSET, 0);
    CC_TileLayer_SetRegister(i, TILE_COLOR_KEY, 0);
  }

  printf("Done with setup.\n");
  while(1);
}