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

#include <stdint.h>
#include <string.h>

#include "cc_core.h"
#include "cc_sprite.h"
#include "cc_tile_layer.h"
#include "registers.h"

// ChronoCube memory map
// TODO: this should be read from dedicated info registers rather than
// hard-coded.
#define REGISTERS_BASE           0x0000
#define TILE_REG_BASE            0x0800
#define PALETTE_BASE             0x1000
#define SPRITE_BASE              0x2000
#define TILEMAP_BASE             0x4000

#define VRAM_BASE                0x4000  // This is banked memory.
#define VRAM_BANK_SIZE           0x4000
#define VRAM_BANK_BASE                2  // VRAM starts at this bank.

#define NUM_SPRITES                 128
#define SPRITE_SIZE                  32
#define SPRITE_LEN        (NUM_SPRITES * SPRITE_SIZE)

#define TILEMAP_SIZE              0x800  // One tilemap is 2 KB.
#define TILEMAP_BANK                  1  // Tilemaps are in this memory bank.

#define NUM_TILE_LAYERS               4
#define TILE_REG_LEN                 16  // Number of registers per tile layer.
#define TILE_REG_ADDR_STEP          128  // Tile layer register address spacing.

#define NUM_PALETTES                  4
#define PALETTE_SIZE              0x400  // Palette is mapped to 1 KB of memory.

#define TILEMAP_LEN              0x2000  // Length of tilemap memory.

// Chronocube is mapped to this address.
// TODO: make this more configurable.
#define MEMORY_BASE              0x8000

static volatile uint16_t* registers;
static volatile uint16_t* tile_regs[NUM_TILE_LAYERS];
static volatile uint8_t* palette;
static volatile uint16_t* palette16;
static volatile uint8_t* vram;
static volatile uint16_t* vram16;
static volatile uint8_t* tilemap;
static volatile uint16_t* tilemap16;

static volatile uint8_t* tilemaps[NUM_TILE_LAYERS];
static volatile uint16_t* tilemaps16[NUM_TILE_LAYERS];

static volatile uint8_t* sprites[NUM_SPRITES];
static volatile uint16_t* sprites16[NUM_SPRITES];

// Initializes ChronoCube memory pointers.
static void InitMemory() {
  registers = (uint16_t*)(MEMORY_BASE + REGISTERS_BASE);

  palette = (uint8_t*)(MEMORY_BASE + PALETTE_BASE);
  palette16 = (uint16_t*)(MEMORY_BASE + PALETTE_BASE);
  vram = (uint8_t*)(MEMORY_BASE + VRAM_BASE);
  vram16 = (uint16_t*)(MEMORY_BASE + VRAM_BASE);
  tilemap = (uint8_t*)(MEMORY_BASE + TILEMAP_BASE);
  tilemap16 = (uint16_t*)(MEMORY_BASE + TILEMAP_BASE);

  int i;
  for (i = 0; i < NUM_TILE_LAYERS; ++i) {
    tile_regs[i] =
        (uint16_t*)(MEMORY_BASE + TILE_REG_BASE + i * TILE_REG_ADDR_STEP);
    tilemaps[i] = (uint8_t*)(MEMORY_BASE + TILEMAP_BASE + i * TILEMAP_SIZE);
    tilemaps16[i] = (uint16_t*)(MEMORY_BASE + TILEMAP_BASE + i * TILEMAP_SIZE);
  }
  for (i = 0; i < NUM_SPRITES; ++i) {
    sprites[i] = (uint8_t*)(MEMORY_BASE + SPRITE_BASE + i * SPRITE_SIZE);
    sprites16[i] = (uint16_t*)(MEMORY_BASE + SPRITE_BASE + i * SPRITE_SIZE);
  }
}

void CC_Init() {
  InitMemory();
}

void CC_Cleanup() {
  // Nothing to be done here on a low-level embedded system.
}

void CC_SetRegister(uint8_t reg, uint16_t value) {
  registers[reg] = value;
}

uint16_t CC_GetRegister(uint8_t reg) {
  return registers[reg];
}

// Tilemap access
void CC_TileLayer_SetData(const void* data,
                          uint8_t index,
                          uint16_t offset,
                          uint16_t size) {
  // Switch memory banks.
  CC_SetRegister(CC_REG_MEM_CTRL, (TILEMAP_BANK << CC_REG_BANK_OFFSET));

  memcpy(tilemaps[index] + offset, data, size);

  CC_SetRegister(CC_REG_MEM_CTRL, 0);
}

void CC_TileLayer_GetData(uint8_t index,
                          uint16_t offset,
                          uint16_t size,
                          void* data) {
  // Switch memory banks.
  CC_SetRegister(CC_REG_MEM_CTRL, (TILEMAP_BANK << CC_REG_BANK_OFFSET));

  memcpy(data, tilemaps[index] + offset, size);

  CC_SetRegister(CC_REG_MEM_CTRL, 0);
}

void CC_TileLayer_SetRegister(uint8_t index, uint8_t reg, uint16_t value) {
  tile_regs[index][reg] = value;
}

uint16_t CC_TileLayer_GetRegister(uint8_t index, uint8_t reg) {
  return tile_regs[index][reg];
}

// Palette access
void CC_SetPaletteData(const void* data,
                       uint8_t index,
                       uint16_t offset,
                       uint16_t size) {
  memcpy(palette + (PALETTE_SIZE * index) + offset, data, size);
}

void CC_GetPaletteData(uint8_t index,
                       uint16_t offset,
                       uint16_t size,
                       void* data) {
  memcpy(data, palette + (PALETTE_SIZE * index) + offset, size);
}

// Sprite access
void CC_Sprite_SetRegister(uint16_t index, uint8_t reg, uint16_t value) {
  sprites16[index][reg] = value;
}

uint16_t CC_Sprite_GetRegister(uint16_t index, uint8_t reg) {
  return sprites16[index][reg];
}

// VRAM access
void CC_SetVRAMData(const void* data, uint32_t offset, uint32_t size) {
  // Align |offset| and |size| to 4 bytes.
  offset = (offset >> 2) << 2;
  size = (size >> 2) << 2;

  const uint8_t* char_data = (const uint8_t*)data;
  uint8_t start_bank = offset / VRAM_BANK_SIZE;

  for (uint32_t bytes_copied = 0; bytes_copied < size; ) {
    // Determine which bank to access, and set the bank.
    uint8_t bank = (offset + bytes_copied) / VRAM_BANK_SIZE + VRAM_BANK_BASE;
    uint16_t bank_offset = (offset + bytes_copied) % VRAM_BANK_SIZE;
    CC_SetRegister(CC_REG_MEM_CTRL,
                   (1 << CC_REG_ENABLE_VRAM_OFFSET) |
                       (bank << CC_REG_BANK_OFFSET));

    uint16_t copy_size = VRAM_BANK_SIZE - bank_offset;
    if (bytes_copied + copy_size > size)
      copy_size = size - bytes_copied;
    memcpy(vram + bank_offset, char_data + bytes_copied, copy_size);
    bytes_copied += copy_size;
  }
  // Reset the memory control register.
  CC_SetRegister(CC_REG_MEM_CTRL, 0);
}

void CC_GetVRAMData(uint32_t offset, uint32_t size, void* data) {
  // Align |offset| and |size| to 4 bytes.
  offset = (offset >> 2) << 2;
  size = (size >> 2) << 2;

  uint8_t* char_data = (uint8_t*)data;
  uint8_t start_bank = offset / VRAM_BANK_SIZE;

  for (uint32_t bytes_copied = 0; bytes_copied < size; ) {
    // Determine which bank to access, and set the bank.
    uint8_t bank = (offset + bytes_copied) / VRAM_BANK_SIZE + VRAM_BANK_BASE;
    uint16_t bank_offset = (offset + bytes_copied) % VRAM_BANK_SIZE;
    CC_SetRegister(CC_REG_MEM_CTRL,
                   (1 << CC_REG_ENABLE_VRAM_OFFSET) | (bank << CC_REG_BANK_OFFSET));

    uint16_t copy_size = VRAM_BANK_SIZE - bank_offset;
    if (bytes_copied + copy_size > size)
      copy_size = size - bytes_copied;
    memcpy(char_data + bytes_copied, vram + bank_offset, copy_size);
    bytes_copied += copy_size;
  }
  // Reset the memory control register.
  CC_SetRegister(CC_REG_MEM_CTRL, 0);
}
