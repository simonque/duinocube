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

// Display functions.

#include "display.h"

#include <string.h>

#include "DuinoCube_core.h"
#include "DuinoCube_defs.h"
#include "DuinoCube_mem.h"
#include "font.h"
#include "printf.h"
#include "shmem.h"

// Track resource usage.
static struct {
  bool initialized;           // Set this flag if the resource has been setup.
  uint8_t layer;              // Tile layer index.
  uint16_t tilemap_addr;      // Cached address of tile layer data.
  uint8_t palette;            // Palette index.
  uint32_t vram_offset;       // VRAM offset.
  uint32_t vram_size;         // VRAM data size.
} g_text_params;    // For text rendering.

static uint32_t g_vram_used;  // Tracks the amount of VRAM that has been used.

// Converts a Core memory address to shared memory space.
#define CORE_TO_SHMEM_ADDR(addr) ((addr) + SHARED_MEMORY_SIZE)

// The tilemap is rendered as 64x32 tiles in 8-bit mode. Each line of the
// tilemap is thus 64 bytes.
#define TEXT_LINE_SIZE         64

// Palette colors.
const uint32_t kPaletteColors[] = {
  0x00000000,   // Black.
  0x00ffffff,   // White.
};

// Functions for accessing core graphics.
static void core_write_data(uint16_t addr, const void* data, uint16_t size) {
  shmem_write(CORE_TO_SHMEM_ADDR(addr), data, size);
}
static void core_write_word(uint16_t addr, uint16_t value) {
  shmem_write(CORE_TO_SHMEM_ADDR(addr), &value, sizeof(value));
}

// Convert text coordinates to a tilemap memory offset.
static inline uint16_t get_text_addr(uint8_t x, uint8_t y) {
  return g_text_params.tilemap_addr + TEXT_LINE_SIZE * y + x;
}

// Initializes text rendering using a particular layer and palette.
void display_text_init(uint8_t layer, uint8_t palette) {
  // For now, support only one text layer.
  if (g_text_params.initialized) {
    return;
  }
  g_text_params.initialized = true;
  g_text_params.layer = layer;
  g_text_params.tilemap_addr = TILEMAP(layer);
  g_text_params.palette = palette;
  g_text_params.vram_offset = g_vram_used;
  g_text_params.vram_size = MAX_FONT_CHARS * FONT_CHAR_SIZE;
  // Increment the VRAM usage counter.
  g_vram_used += g_text_params.vram_size;

  // TODO: Reset the Core.

  uint8_t buf[FONT_CHAR_SIZE];

  // Load font.
  core_write_word(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));
  core_write_word(REG_MEM_BANK, VRAM_BANK_BEGIN);
  uint16_t offset = VRAM_BASE;
  for (uint8_t ch = 0; ch < MAX_FONT_CHARS; ++ch) {
    // Load each character's bitmap individually.
    font_load_bitmap(ch, buf);
    core_write_data(offset, buf, FONT_CHAR_SIZE);
    offset += FONT_CHAR_SIZE;
  }
  core_write_word(REG_SYS_CTRL, (0 << REG_SYS_CTRL_VRAM_ACCESS));

  // Set up palette.
  core_write_data(PALETTE(palette), kPaletteColors, sizeof(kPaletteColors));

  // Clear tilemap of the text layer.
  memset(buf, 0, sizeof(buf));
  core_write_word(REG_MEM_BANK, TILEMAP_BANK);
  for (uint16_t offset = 0; offset < TILEMAP_SIZE; offset += sizeof(buf)) {
    core_write_data(g_text_params.tilemap_addr + offset, buf, sizeof(buf));
  }

  // Enable the tile layer.
  core_write_word(TILE_LAYER_REG(layer, TILE_CTRL_0),
                  (1 << TILE_LAYER_ENABLED) |
                  (1 << TILE_ENABLE_8x8) |
                  (1 << TILE_ENABLE_8_BIT) |
                  (1 << TILE_SHIFT_DATA_OFFSET) |
                  (palette << TILE_PALETTE_START));
  core_write_word(TILE_LAYER_REG(layer, TILE_DATA_OFFSET),
                  LARGE_VRAM_DATA_OFFSET(g_text_params.vram_offset));
}

// Clears |length| characters at the given location.
void display_text_clear(uint8_t len, uint8_t x, uint8_t y) {
  uint8_t buf[256];
  memset(buf, 0, len);
  core_write_data(get_text_addr(x, y), buf, len);
}

// Renders text at the given location.
void display_text_render(const char* text, uint8_t x, uint8_t y) {
  core_write_data(get_text_addr(x, y), text, strlen(text));
}

void display_text_render_P(const char* text, uint8_t x, uint8_t y) {
  char buf[TEXT_LINE_SIZE];
  strncpy_P(buf, text, sizeof(buf));
  display_text_render(buf, x, y);
}
