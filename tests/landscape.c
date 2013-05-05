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

// Tile layer and sprite drawing demo.

#include <stdio.h>
#include <string.h>

#include "cc_core.h"
#include "cc_tile_layer.h"
#include "system.h"

#include "registers.h"
#include "sprite_registers.h"
#include "tile_registers.h"

#include "data/tileset.raw.h"
#include "data/tileset.pal.h"

#include "data/desert.tmx.layer0.h"
#include "data/desert.tmx.layer1.h"
#include "data/desert.tmx.layer2.h"

#include "data/cloud.pal.h"
#include "data/cloud.raw.h"
#include "data/clouds.tmx.layer0.h"

#include "data/cecil.raw.h"
#include "data/cecil.pal.h"

#define PALETTE_SIZE    1024
#define NUM_PALETTES       3
static void setup_palettes(void) {
  uint16_t i, palette;
  uint8_t buf[PALETTE_SIZE];
  const uint8_t* palettes[NUM_PALETTES] = {
      palette_data,
      cloud_bmp_pal_data8,
      cecil_bmp_pal_data8
  };

  for (palette = 0; palette < NUM_PALETTES; ++palette) {
    for (i = 0; i < PALETTE_SIZE; ++i)
      buf[i] = pgm_read_byte(palettes[palette] + i);
    CC_SetPaletteData(buf, palette, 0, PALETTE_SIZE);
  }
}

#define TILEMAP_LENGTH    2048
#define NUM_TILEMAPS         4
static void setup_tilemaps(void) {
  uint8_t buf[TILEMAP_LENGTH];
  uint16_t i, tilemap;
  const uint8_t* tilemaps[NUM_TILEMAPS] = {
      desert_tmx_layer0_dat_data16,
      desert_tmx_layer1_dat_data16,
      desert_tmx_layer2_dat_data16,
      clouds_tmx_layer0_dat_data16,
  };
  for (tilemap = 0; tilemap < NUM_TILEMAPS; ++tilemap) {
    for (i = 0; i < TILEMAP_LENGTH; ++i)
      buf[i] = pgm_read_word(tilemaps[tilemap] + i);
    CC_TileLayer_SetData(buf, tilemap, 0, TILEMAP_LENGTH);
  }
}

#define VRAM_COPY_SIZE  1024
static void copy_vram_pgm(const void* source, uint32_t offset, uint16_t size) {
  uint8_t buf[VRAM_COPY_SIZE];
  uint16_t i;
  uint32_t bytes_copied;
  for (bytes_copied = 0; bytes_copied < size; ) {
    uint32_t copy_size;
    if (bytes_copied + VRAM_COPY_SIZE <= size)
      copy_size = VRAM_COPY_SIZE;
    else
      copy_size = size - bytes_copied;

    for (i = 0; i < copy_size; ++i)
      buf[i] = pgm_read_byte(((const uint8_t*)source) + bytes_copied + i);
    CC_SetVRAMData(buf, offset + bytes_copied, copy_size);

    bytes_copied += copy_size;
  }
}

static void setup_vram(void) {
  uint32_t offset = 0;
  copy_vram_pgm(tileset8_bmp_raw_data8, offset, TILESET8_BMP_RAW_DATA_SIZE);
  offset += TILESET8_BMP_RAW_DATA_SIZE;

  copy_vram_pgm(cloud_bmp_raw_data8, offset, CLOUD_BMP_RAW_DATA_SIZE);
  offset += CLOUD_BMP_RAW_DATA_SIZE;

  copy_vram_pgm(cecil_bmp_raw_data8, offset, CECIL_BMP_RAW_DATA_SIZE);
  offset += CECIL_BMP_RAW_DATA_SIZE;
}

int main (void) {
  system_init();
  CC_Init();

  setup_palettes();
  setup_tilemaps();
  setup_vram();

  uint16_t tile_ctrl0_value =
      (1 << TILE_LAYER_ENABLED) |
      (1 << TILE_ENABLE_NOP) |
      (1 << TILE_ENABLE_TRANSP) |
      (1 << TILE_ENABLE_FLIP);
  uint16_t i;
  for (i = 0; i < NUM_TILE_LAYERS; ++i) {
    CC_TileLayer_SetRegister(i, TILE_CTRL0, tile_ctrl0_value);
    CC_TileLayer_SetRegister(i, TILE_NOP_VALUE, 0x1fff);
    CC_TileLayer_SetRegister(i, TILE_COLOR_KEY, 0xff);
  }
  CC_TileLayer_SetRegister(3, TILE_DATA_OFFSET, TILESET8_BMP_RAW_DATA_SIZE);
  CC_TileLayer_SetRegister(3, TILE_CTRL0, tile_ctrl0_value | (1 << 12));

  // Draw some sprites in various orientations.
#define NUM_SPRITES_DRAWN 8
  for (i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    uint16_t sprite_ctrl0_value = (1 << SPRITE_ENABLED) |
                                  (1 << SPRITE_ENABLE_TRANSP) |
                                  ((i & 1) << SPRITE_ENABLE_SCROLL) |
                                  (2 << SPRITE_PALETTE_START);
    if (i & 1)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_X);
    if (i & 2)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_Y);
    if (i & 4)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_XY);

    CC_Sprite_SetRegister(i, SPRITE_CTRL0, sprite_ctrl0_value);
    CC_Sprite_SetRegister(i, SPRITE_CTRL1,
                          (1 << SPRITE_HSIZE_0) | (1 << SPRITE_VSIZE_1));
    CC_Sprite_SetRegister(i, SPRITE_DATA_OFFSET, 0x4000 + 16 * 32);
    CC_Sprite_SetRegister(i, SPRITE_COLOR_KEY, 0xff);
    CC_Sprite_SetRegister(i, SPRITE_OFFSET_X, i * 3 - 10);
    CC_Sprite_SetRegister(i, SPRITE_OFFSET_Y, i * 32 + 512 - 16);
  }

  CC_SetRegister(CC_REG_SPRITE_Z, 3);

  printf("Done with setup\n");

  if (1) {
    int step = 16;
      for (i = 0; ; i += step) {
      while(!(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK)));
      CC_SetRegister(CC_REG_SCROLL_X, (i / 32) % 512);
      CC_SetRegister(CC_REG_SCROLL_Y, (i / 16) % 512);

      CC_TileLayer_SetRegister(3, TILE_OFFSET_X, i / 16);
      CC_TileLayer_SetRegister(3, TILE_OFFSET_Y, -i / 32);
      while(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK));
    }
  }

  return 0;
}