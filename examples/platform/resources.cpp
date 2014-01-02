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

// Contains function to load resource files.

#include "resources.h"

#include <stdio.h>

#include <avr/pgmspace.h>

#include <DuinoCube.h>

struct File {
  const char* filename;
  uint16_t* vram_offset;  // For image data, compute and store VRAM offset here.
  uint16_t addr;          // For other data, store data here.
  uint16_t bank;          // Bank to use for |addr|.
  uint16_t max_size;      // Size checking to avoid overflow.
};

// VRAM offsets of background and sprite image data.
uint16_t g_bg_offset;
uint16_t g_moon_offset;

namespace {

const char kFilePath[] = "platform";    // Base path of data files.

const File kFiles[] = {
  // Tilemap files.
  { "bg.lay", NULL, TILEMAP(BG_TILEMAP_INDEX), TILEMAP_BANK, TILEMAP_SIZE },
  { "moon.lay", NULL, TILEMAP(MOON_TILEMAP_INDEX), TILEMAP_BANK, TILEMAP_SIZE },

  // Image files.
  { "bg.raw", &g_bg_offset, 0, 0, VRAM_BANK_SIZE },
  { "moon.raw", &g_moon_offset, 0, 0, VRAM_BANK_SIZE },

  // Palette files.
  { "bg.pal", NULL, PALETTE(BG_PALETTE_INDEX), 0, PALETTE_SIZE },
  { "moon.pal", NULL, PALETTE(MOON_PALETTE_INDEX), 0, PALETTE_SIZE },
};

}  // namespace

// Load image, palette, and tilemap data from file system.
void loadResources() {
  uint16_t vram_offset = 0;
  for (int i = 0; i < sizeof(kFiles) / sizeof(kFiles[0]); ++i) {
    const File& file = kFiles[i];

    char filename[256];
    sprintf(filename, "%s/%s", kFilePath, file.filename);

    // Open the file.
    uint16_t handle = DC.File.open(filename, FILE_READ_ONLY);
    if (!handle) {
      printf_P(PSTR("Could not open file %s.\n"), filename);
      continue;
    }

    uint16_t file_size = DC.File.size(handle);
    printf_P(PSTR("File %s is 0x%x bytes\n"), filename, file_size);

    if (file_size > file.max_size) {
      printf_P(PSTR("File is too big!\n"));
      DC.File.close(handle);
      continue;
    }

    // Compute write destination.
    uint16_t dest_addr;
    uint16_t dest_bank;

    if (file.vram_offset) {
      // Set up for VRAM write.

      // If this doesn't fit in the remaining part of the current bank, use the
      // next VRAM bank.
      if (vram_offset % VRAM_BANK_SIZE + file_size > VRAM_BANK_SIZE)
        vram_offset += VRAM_BANK_SIZE - (vram_offset % VRAM_BANK_SIZE);

      // Record VRAM offset.
      *file.vram_offset = vram_offset;

      // Determine the destination VRAM address and bank.
      dest_addr = VRAM_BASE + vram_offset % VRAM_BANK_SIZE;
      dest_bank = vram_offset / VRAM_BANK_SIZE + VRAM_BANK_BEGIN;
      DC.Core.writeWord(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));

      // Update the VRAM offset.
      vram_offset += file_size;
    } else {
      // Set up for non-VRAM write.
      dest_addr = file.addr;
      dest_bank = file.bank;
    }

    printf_P(PSTR("Writing to 0x%x with bank = %d\n"), dest_addr, dest_bank);
    DC.Core.writeWord(REG_MEM_BANK, dest_bank);
    DC.File.readToCore(handle, dest_addr, file_size);

    DC.File.close(handle);
  }

  // Set to bank 0.
  DC.Core.writeWord(REG_MEM_BANK, 0);

  // Allow the graphics pipeline access to VRAM.
  DC.Core.writeWord(REG_SYS_CTRL, (0 << REG_SYS_CTRL_VRAM_ACCESS));
}

void setupLayers() {
  for (int layer_index = 0; layer_index < NUM_TILE_LAYERS; ++layer_index) {
    uint16_t data_offset = 0;
    uint8_t palette_index = 0;
    bool is_transparent = false;
    switch (layer_index) {
    case BG_TILEMAP_INDEX:
      data_offset = g_bg_offset;
      palette_index = BG_PALETTE_INDEX;
      break;
    case MOON_TILEMAP_INDEX:
      data_offset = g_moon_offset;
      palette_index = MOON_PALETTE_INDEX;
      is_transparent = true;
      break;
    default:
      // Disable other layers.
      DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_CTRL_0), 0);
      continue;
    }

    DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_CTRL_0),
                      (1 << TILE_LAYER_ENABLED) |
                      (1 << TILE_ENABLE_NOP) |
                      (1 << TILE_ENABLE_FLIP) |
                      (is_transparent << TILE_ENABLE_TRANSP) |
                      (palette_index << TILE_PALETTE_START));
    DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_DATA_OFFSET),
                      data_offset);
    DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_EMPTY_VALUE),
                      DEFAULT_EMPTY_TILE_VALUE);
  }
}
