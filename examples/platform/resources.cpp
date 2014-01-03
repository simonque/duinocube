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
uint16_t g_level_offset;

// Shared memory buffer containing level data.
uint16_t g_level_buffer;

namespace {

const char kFilePath[] = "platform";    // Base path of data files.

const File kFiles[] PROGMEM = {
  // Tilemap files.
  { "bg.lay", NULL, TILEMAP(BG_TILEMAP_INDEX), TILEMAP_BANK, TILEMAP_SIZE },
  { "moon.lay", NULL, TILEMAP(MOON_TILEMAP_INDEX), TILEMAP_BANK, TILEMAP_SIZE },

  // Image files.
  { "bg.raw", &g_bg_offset, 0, 0, ~(uint16_t)(0) },
  { "moon.raw", &g_moon_offset, 0, 0, ~(uint16_t)(0) },
  { "twilight.raw", &g_level_offset, 0, 0, ~(uint16_t)(0) },

  // Palette files.
  { "bg.pal", NULL, PALETTE(BG_PALETTE_INDEX), 0, PALETTE_SIZE },
  { "moon.pal", NULL, PALETTE(MOON_PALETTE_INDEX), 0, PALETTE_SIZE },
  { "twilight.pal", NULL, PALETTE(LEVEL_PALETTE_INDEX), 0, PALETTE_SIZE },
};

const char kLevelFile[] = "level.lay";

// Copies data from a file to DuinoCube core. The file must already be open with
// a valid handle.
void copyFileDataToCore(uint16_t handle, uint16_t addr, uint16_t bank,
                        uint16_t size) {
  printf_P(PSTR("Writing 0x%x bytes to 0x%x with bank = %d\n"),
           size, addr, bank);
  DC.Core.writeWord(REG_MEM_BANK, bank);
  DC.File.readToCore(handle, addr, size);
}

// Opens a file, reports on the status, and returns a handle.
uint16_t openFile(const char* base_filename) {
  char filename[256];
  sprintf(filename, "%s/%s", kFilePath, base_filename);

  // Open the file.
  uint16_t handle = DC.File.open(filename, FILE_READ_ONLY);
  if (!handle) {
    printf_P(PSTR("Could not open file %s.\n"), filename);
    return handle;
  }

  printf_P(PSTR("File %s is 0x%x bytes\n"), filename, DC.File.size(handle));
  return handle;
}

// Open a level file and store it in memory.
// Returns NULL on failure.
uint16_t loadLevel(const char* base_filename) {
  uint16_t handle = openFile(base_filename);
  if (!handle) {
    return NULL;
  }
  uint16_t size = LEVEL_SIZE;

  // Allocate memory.
  uint16_t level_buffer = DC.Mem.alloc(size);
  if (!level_buffer) {
    printf_P(PSTR("Unable to allocate 0x%x bytes.\n"), size);
    return NULL;
  }

  // Read file contents into it.
  uint16_t size_read = DC.File.read(handle, level_buffer, size);
  if (size_read < size) {
    printf_P(PSTR("Only read 0x%x bytes from file.\n"), size_read);
  }

  // Copy the part of the level map to tilemap memory.
  DC.Core.writeWord(REG_MEM_BANK, TILEMAP_BANK);
  for (int y = 0; y < LEVEL_HEIGHT && y < TILEMAP_HEIGHT; ++y) {
    uint16_t level_offset = y * LEVEL_WIDTH * TILEMAP_ENTRY_SIZE;
    uint16_t tilemap_offset = y * TILEMAP_WIDTH * TILEMAP_ENTRY_SIZE;

    uint8_t tilemap_line[TILEMAP_WIDTH * TILEMAP_ENTRY_SIZE];
    DC.Sys.readSharedRAM(level_buffer + level_offset, tilemap_line,
                         sizeof(tilemap_line));
    DC.Core.writeData(TILEMAP(LEVEL_TILEMAP_INDEX) + tilemap_offset,
                      tilemap_line, sizeof(tilemap_line));
  }

  return level_buffer;
}

}  // namespace

// Load image, palette, and tilemap data from file system.
void loadResources() {
  uint16_t vram_offset = 0;
  for (int i = 0; i < sizeof(kFiles) / sizeof(kFiles[0]); ++i) {
    // Copy from program memory.
    File file;
    memcpy_P(&file, kFiles + i, sizeof(file));

    // Open the file.
    uint16_t handle = openFile(file.filename);
    if (!handle) {
      continue;
    }
    uint16_t file_size = DC.File.size(handle);

    // Set up for VRAM write.
    DC.Core.writeWord(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));

    if (file.vram_offset) {
      // Record VRAM offset.
      *file.vram_offset = vram_offset;

      // Determine how much space remains on the current bank.
      uint16_t bank_size_left = VRAM_BANK_SIZE - (vram_offset % VRAM_BANK_SIZE);
      uint16_t size_to_copy =
          (file_size > bank_size_left) ? bank_size_left : file_size;
      while (file_size > 0) {
        // Determine the destination VRAM address and bank.
        uint16_t dest_addr = VRAM_BASE + vram_offset % VRAM_BANK_SIZE;
        uint16_t dest_bank = vram_offset / VRAM_BANK_SIZE + VRAM_BANK_BEGIN;

        copyFileDataToCore(handle, dest_addr, dest_bank, size_to_copy);

        // Increment VRAM offset counter.
        vram_offset += size_to_copy;
        file_size -= size_to_copy;

        // Compute size of next chuck to copy.
        size_to_copy =
            (file_size > VRAM_BANK_SIZE) ? VRAM_BANK_SIZE : file_size;
      }
    } else {
      // Set up for non-VRAM write.
      copyFileDataToCore(handle, file.addr, file.bank, file_size);
    }

    DC.File.close(handle);
  }

  g_level_buffer = loadLevel(kLevelFile);

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
    case LEVEL_TILEMAP_INDEX:
      data_offset = g_level_offset;
      palette_index = LEVEL_PALETTE_INDEX;
      is_transparent = true;
      break;
    default:
      // Disable other layers.
      DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_CTRL_0), 0);
      continue;
    }

    printf_P(PSTR("Enabling layer with data offset 0x%x\n"), data_offset);

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
