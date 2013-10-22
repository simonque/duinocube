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

// Test the DuinoCube tile layer drawing, scrolling, and transparency.

#include <DuinoCube.h>
#include <SPI.h>

// Files to load.
const char* image_files[] = {
  "data/tileset.raw",
  "data/clouds.raw",
};

const char* palette_files[] = {
  "data/tileset.pal",
  "data/clouds.pal",
};

const char* layer_files[] = {
  "data/desert0.lay",
  "data/desert1.lay",
  "data/desert2.lay",
  "data/clouds.lay",
};

static void draw() {
  DC.Core.writeWord(REG_MEM_BANK, 0);
  DC.Core.writeWord(REG_SYS_CTRL, 0);
  for (int i = 0; i < 4; ++i) {
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_CTRL_0), 0);
  }

  // Load palettes.
  printf("Loading palettes.\n");
  for (int i = 0; i < sizeof(palette_files) / sizeof(palette_files[0]); ++i) {
    const char* filename = palette_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, PALETTE(i), file_size), PALETTE(i));
    DC.File.close(handle);
  }

  // Load layers.
  printf("Loading layers.\n");
  printf("Layers: %d\n", sizeof(layer_files) / sizeof(layer_files[0]));
  for (int i = 0; i < sizeof(layer_files) / sizeof(layer_files[0]); ++i) {
    const char* filename = layer_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    DC.Core.writeWord(REG_MEM_BANK, TILEMAP_BANK);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, TILEMAP(i), file_size), TILEMAP(i));
    DC.Core.writeWord(REG_MEM_BANK, 0);
    DC.File.close(handle);
  }

  // Load images.
  printf("Loading images.\n");
  uint16_t addr = VRAM_BASE;
  for (int i = 0; i < sizeof(image_files) / sizeof(image_files[0]); ++i) {
    const char* filename = image_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    DC.Core.writeWord(REG_MEM_BANK, VRAM_BANK_BEGIN);
    DC.Core.writeWord(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, addr, file_size), addr);
    DC.Core.writeWord(REG_MEM_BANK, 0);
    DC.Core.writeWord(REG_SYS_CTRL, 0);
    addr += file_size;
    DC.File.close(handle);
  }

  // Turn off sprites so they don't interfere with the landscape demo.
  for (int i = 0; i < NUM_SPRITES; ++i)
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0), 0);

  // Set up tile layer registers.
  // TODO: explain value.
  word tile_ctrl0_value =
      (1 << TILE_LAYER_ENABLED) |
      (1 << TILE_ENABLE_NOP) |
      (1 << TILE_ENABLE_TRANSP) |
      (1 << TILE_ENABLE_FLIP);

  struct TileLayerInfo {
    uint16_t ctrl0, empty, color_key, offset;
  };

  const TileLayerInfo layer_info[] = {
    // TODO: explain the meaning of the values.
    { tile_ctrl0_value, 0x1fff, 0xff, 0 },
    { tile_ctrl0_value, 0x1fff, 0xff, 0 },
    { tile_ctrl0_value, 0x1fff, 0xff, 0 },
    { tile_ctrl0_value | 0x1000, 0x1fff, 0xff, 0x3000 },
  };

  for (int i = 0; i < sizeof(layer_files) / sizeof(layer_files[0]); ++i) {
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_CTRL_0), layer_info[i].ctrl0);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_EMPTY_VALUE), layer_info[i].empty);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_COLOR_KEY),
                      layer_info[i].color_key);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_DATA_OFFSET),
                      layer_info[i].offset);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_OFFSET_X), 0);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_OFFSET_Y), 0);
  }
}

void setup() {
  Serial.begin(115200);

  DC.begin();

  draw();
}

void loop() {
  // Start camera at (0, 0).
  DC.Core.writeWord(REG_SCROLL_X, 0);
  DC.Core.writeWord(REG_SCROLL_Y, 0);

  const int step = 8;
  for (uint16_t i = 0; ; i += step) {
    // Wait for the start of the next Vblank.  So first wait for non-Vblanking,
    // and then wait for Vblank.
    while (DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK));

    // Calculate the next scroll values during the non-Vblank area, where there
    // is more time.
    uint16_t scroll_x = (i / 16) % 512;
    uint16_t scroll_y = (i / 8) % 512;
    uint16_t clouds_x = (i / 8);
    uint16_t clouds_y = -(i / 16);

    while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK))) ;

    // Scroll the camera.
    DC.Core.writeWord(REG_SCROLL_X, scroll_x);
    DC.Core.writeWord(REG_SCROLL_Y, scroll_y);

    // Scroll the cloud layer independently.
    DC.Core.writeWord(TILE_LAYER_REG(3, TILE_OFFSET_X), clouds_x);
    DC.Core.writeWord(TILE_LAYER_REG(3, TILE_OFFSET_Y), clouds_y);
  }
}
