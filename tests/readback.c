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

// Memory readback test.

#include <stdio.h>
#include <string.h>

#include "cc_core.h"
#include "cc_sprite.h"
#include "cc_tile_layer.h"

#include "system.h"

// Use a smaller buffer to read/write long blocks of memory in ChronoCube.
// This value may need to be tuned based on available memory in a system.
#define BUF_SIZE             256

// Do not keep testing after enough errors have been encountered.  Otherwise
// there's too much logging spew.
#define MAX_ERRORS_PER_TEST   16

// Used for generating pseudorandom data values for testing.
static uint16_t get_test_value(uint16_t in) {
  uint16_t prod = (in * in + 1357) % 2913;
  int shift = prod & 7;
  prod = (prod << shift) | (prod >> (16-shift));
  prod ^= 0xaaaa;
  return prod;
}

#define NUM_REGISTERS      16
#define NUM_TILE_LAYERS     4
#define TILE_REG_LEN       16

static void test_registers() {
  printf("Reading registers.\n");
  uint8_t reg;

  for (reg = 0; reg < NUM_REGISTERS; ++reg)
    CC_SetRegister(reg, get_test_value(reg));

  for (reg = 0; reg < NUM_REGISTERS; ++reg) {
    uint16_t value = CC_GetRegister(reg);
    if (value != get_test_value(reg)) {
      printf("Read 0x%04x from REGISTERS[0x%04x], expecting 0x%04x\n",
             value, reg, get_test_value(reg));
    }
    // Reset register.
    CC_SetRegister(reg, 0);
  }

  uint8_t i;

  // TODO: All tile registers will have to be tested eventually.  However, not
  // all of them are currently being used, so they're left out of the FPGA
  // implementation to speed up the build.
  uint8_t read_reg_flags[TILE_REG_LEN];
  memset(read_reg_flags, 0, sizeof(read_reg_flags));
  read_reg_flags[0] = 1;
  read_reg_flags[3] = 1;
  read_reg_flags[4] = 1;
  read_reg_flags[5] = 1;

  for (i = 0; i < NUM_TILE_LAYERS; ++i) {
    for (reg = 0; reg < TILE_REG_LEN; ++reg)
      CC_TileLayer_SetRegister(i, reg, get_test_value(i + reg));

    for (reg = 0; reg < TILE_REG_LEN; ++reg) {
      if (!read_reg_flags[reg])
        continue;
      uint16_t offset = i + reg;
      uint16_t value = CC_TileLayer_GetRegister(i, reg);
      if (value != get_test_value(offset))
        printf("Read 0x%04x from TILE_REGS[%d][0x%04x], expecting 0x%04x\n",
               value, i, reg, get_test_value(offset));

      // Reset register.
      CC_TileLayer_SetRegister(i, reg, 0);
    }
  }
}

#define NUM_PALETTES      4
#define PALETTE_SIZE   1024

static void test_palette() {
  printf("Testing palette.\n");
  uint16_t i, x, offset;
  uint16_t num_errors = 0;
  uint8_t buf[BUF_SIZE];

  // For each palette.
  for (i = 0; i < NUM_PALETTES; ++i) {
    // For each |BUF_SIZE| chunk.
    for (offset = 0; offset < PALETTE_SIZE; offset += BUF_SIZE) {
      // For each byte in the buffer.
      for (x = 0; x < BUF_SIZE; ++x)
        buf[x] = get_test_value(i + offset + x);
      CC_SetPaletteData(buf, i, offset * BUF_SIZE, BUF_SIZE);

      // Clear the buffer and read back the data.
      memset(buf, 0, BUF_SIZE * sizeof(buf[0]));
      CC_GetPaletteData(i, offset * BUF_SIZE, BUF_SIZE, buf);

      for (x = 0; x < BUF_SIZE; x++) {
        uint8_t value = buf[x];
        uint8_t expected = get_test_value(i + offset + x);
        // Every fourth byte should be zero, since only three bytes are used for
        // RGB.
        if ((x % 4) == 3) {
          if (value != 0) {
            printf("Read %u from PALETTE[0x%04x], expecting %u\n",
                   value, x + i * PALETTE_SIZE, 0);
            ++num_errors;
          }
        } else if (value != expected) {
          printf("Read %u from PALETTE[0x%04x], expecting %u\n",
                 value, x + i * PALETTE_SIZE, expected);
          ++num_errors;
        }
        if (num_errors >= MAX_ERRORS_PER_TEST)
          return;
      }
    }
  }
}

#define NUM_SPRITES           128
#define NUM_REGS_PER_SPRITE    16

static void test_sprites() {
  printf("Testing sprites.\n");
  uint16_t reg, i;
  uint16_t num_errors = 0;
  for (i = 0; i < NUM_SPRITES; ++i) {
    for (reg = 0; reg < NUM_REGS_PER_SPRITE; reg++)
      CC_Sprite_SetRegister(i, reg, get_test_value(reg + i));

    for (reg = 0; reg < NUM_REGS_PER_SPRITE; reg++) {
      uint16_t value = CC_Sprite_GetRegister(i, reg);
      CC_Sprite_SetRegister(i, reg, 0);
      if (value == get_test_value(reg + i))
        continue;

      printf("Read 0x%04x from SPRITES[0x%02x][0x%04x], expecting 0x%04x\n",
             value, i, reg, get_test_value(reg + i));
      ++num_errors;
      if (num_errors >= MAX_ERRORS_PER_TEST)
        return;
    }
  }
}

#define TILEMAP_SIZE            0x800

static void test_tilemaps() {
  printf("Testing tilemaps.\n");
  uint16_t i, x, offset, num_errors = 0;
  uint8_t buf[BUF_SIZE];

  for (i = 0; i < NUM_TILE_LAYERS; ++i) {
    for (offset = 0; offset < TILEMAP_SIZE; offset += BUF_SIZE) {
      for(x = 0; x < BUF_SIZE; ++x)
        buf[x] = get_test_value(i + offset + x);

      CC_TileLayer_SetData(buf, i, offset, BUF_SIZE);

      memset(buf, 0, BUF_SIZE * sizeof(buf[0]));
      CC_TileLayer_GetData(i, offset, BUF_SIZE, buf);

      for (x = 0; x < BUF_SIZE; x++) {
        uint8_t value = buf[x];
        uint8_t expected = get_test_value(i + offset + x);
        if (value != expected) {
          printf("Read %u from TILEMAP[%d][0x%04x], expected %d\n",
                 value, i, offset + x, expected);
          ++num_errors;
          if (num_errors >= MAX_ERRORS_PER_TEST)
            return;
        }
      }
    }
  }
}

#define VRAM_TEST_LEN           0x400
#define VRAM_TEST_BANK_BEGIN        2
#define VRAM_TEST_BANK_END          4

static void test_vram() {
  printf("Testing VRAM.\n");

  uint16_t x, offset, num_errors = 0;
  uint8_t buf[BUF_SIZE];

  for (offset = 0; offset < VRAM_TEST_LEN; offset += BUF_SIZE) {
    for (x = 0; x < BUF_SIZE; ++x)
      buf[x] = get_test_value(x + offset);

    CC_SetVRAMData(buf, offset, BUF_SIZE);

    memset(buf, 0, BUF_SIZE * sizeof(buf[0]));
    CC_GetVRAMData(offset, BUF_SIZE, buf);

    for (x = 0; x < BUF_SIZE; ++x) {
      uint8_t expected = get_test_value(x + offset);
      if (buf[x] != expected) {
        printf("Read 0x%04x from VRAM[0x%04x], expecting 0x%04x\n",
               buf[x], x + offset, expected);
        ++num_errors;
        if (num_errors >= MAX_ERRORS_PER_TEST)
          return;
      }
    }
  }
}

int main (void) {

  system_init();
  CC_Init();

  test_registers();
  test_palette();
  test_sprites();
  test_tilemaps();
  test_vram();

  printf("Done with testing.\n");

  return 0;
}
