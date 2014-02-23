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

// DuinoCube memory and register definitions.

#ifndef __DUINOCUBE_CORE_DEFS_H__
#define __DUINOCUBE_CORE_DEFS_H__

#include <stdint.h>

// Size of registers in bits.
#define REG_WIDTH              16

// Main registers.
#define REG_ID             0x0000
#define REG_OUTPUT_STATUS  0x0002
#define REG_SCAN_X         0x0004
#define REG_SCAN_Y         0x0006
#define REG_SYS_CTRL       0x0008
#define REG_MEM_BANK       0x000a
#define REG_OUTPUT_CTRL    0x000c
#define REG_MODE_CTRL      0x000e

#define REG_SPRITE_Z       0x0010
#define REG_SCROLL_X       0x001c
#define REG_SCROLL_Y       0x001e

// SYS_CTRL register fields.
#define REG_SYS_CTRL_VRAM_ACCESS      0     // Enable MPU access to VRAM.
#define REG_SYS_CTRL_RESET           15     // Device reset.

// Collision table.
#define COLL_BASE          0x0200

// These bits indicate sprite collisions, one bit per sprite.
#define COLL_STATUS_REG_BASE    0
#define NUM_COLL_STATUS_REGS   32
// Register to clear the sprite collision status bits.
#define COLL_REGS_CLEAR        NUM_COLL_STATUS_REGS
// Generate the address of a collision register.
#define COLL_REG(reg)        (COLL_BASE + reg)

// The collision table contains info about which sprites collided.
#define COLL_TABLE_BASE    0x0100
#define COLL_TABLE_SIZE    0x0100
#define COLL_TABLE_ENTRY(i)  (COLL_BASE + COLL_TABLE_BASE + i)

// Tile layer registers.
#define TILE_REG_BASE      0x0100
#define NUM_TILE_REGISTERS     16
#define TILE_REG_STRIDE        64  // Tile layer registers are spaced out in
                                   // 32-register blocks.
#define NUM_TILE_LAYERS         4
#define TILE_LAYER_REG(index, reg)  \
            (TILE_REG_BASE + (index) * TILE_REG_STRIDE + (reg) * 2)
#define TILE_CTRL_0           0x0
#define TILE_CTRL_1           0x1
#define TILE_DATA_OFFSET      0x3
#define TILE_EMPTY_VALUE      0x4
#define TILE_COLOR_KEY        0x5
#define TILE_OFFSET_X         0x8
#define TILE_OFFSET_Y         0x9

// Palette.
#define PALETTE_BASE       0x1000
#define PALETTE_SIZE       0x0400  // Length of a palette in bytes.
#define NUM_PALETTE_ENTRIES   256
#define NUM_PALETTES            4
#define PALETTE_ENTRY_SIZE      4
#define PALETTE(index)    (PALETTE_BASE + (index) * PALETTE_SIZE)
#define PALETTE_ENTRY(index, entry) \
            (PALETTE(index) + (entry) * PALETTE_ENTRY_SIZE)

// Sprites.
#define SPRITE_BASE        0x2000
#define NUM_SPRITES           256
#define NUM_SPRITE_REGS        16  // Number of registers per sprite.
#define SPRITE_XY_BASE     0x0400  // Address of block of X/Y alias registers.

#define SPRITE_REG(index, reg)  \
            (SPRITE_BASE + (index) * NUM_SPRITE_REGS * 2 + (reg) * 2)
#define SPRITE_CTRL_0         0x0
#define SPRITE_CTRL_1         0x1
#define SPRITE_DATA_OFFSET    0x3
#define SPRITE_REF_XY         0x4
#define SPRITE_COLOR_KEY      0x5
#define SPRITE_OFFSET_X       0x8
#define SPRITE_OFFSET_Y       0x9

// Tile maps.
#define TILEMAP_BASE       0x4000
#define NUM_TILEMAPS            4
#define TILEMAP_SIZE       0x0800  // Size of tilemap data in bytes.
#define TILEMAP(index)    (TILEMAP_BASE + (index) * TILEMAP_SIZE)
#define TILEMAP_BANK            1  // Tilemaps are in this memory bank.

// VRAM definitions.
#define VRAM_BASE          0x4000
#define VRAM_BANK_SIZE     0x4000
#define NUM_VRAM_BANKS         32
#define VRAM_BANK_BEGIN         2  // VRAM spans several 16-KB banks.
#define VRAM_BANK_END     (VRAM_BANK_BEGIN + NUM_VRAM_BANKS)
// This macro computes the memory bank and offset within that bank, given a VRAM
// offset.
#define GET_VRAM_BANK(offset) \
    (((uint32_t)(offset)) / VRAM_BANK_SIZE + VRAM_BANK_BEGIN)
#define GET_VRAM_BANK_OFFSET(offset) \
    (((uint32_t)(offset)) % VRAM_BANK_SIZE)

// When the *_SHIFT_DATA_OFFSET bit is set, the corresponding *_DATA_OFFSET
// register value will be interpreted as the actual address shifted left by
// this amount.
// e.g. a register value of 0x1234 means the data is at (0x1234 << 6) = 0x48d00.
#define VRAM_DATA_OFFSET_SHIFT  6
#define LARGE_VRAM_DATA_OFFSET(offset) ((offset) >> VRAM_DATA_OFFSET_SHIFT)

// ==== Main register fields ====
// TODO: Add more defs here.

// REG_OUTPUT_STATUS
#define REG_HSYNC               0
#define REG_VSYNC               1
#define REG_HBLANK              2
#define REG_VBLANK              3


// ==== Tile layer register fields ====

// TILE_CTRL0
#define TILE_LAYER_ENABLED       0
#define TILE_ENABLE_8x8          1
#define TILE_ENABLE_8_BIT        2
#define TILE_ENABLE_NOP          3
#define TILE_ENABLE_SCROLL       4
#define TILE_ENABLE_TRANSP       5
#define TILE_ENABLE_ALPHA        6
#define TILE_ENABLE_COLOR        7
#define TILE_ENABLE_WRAP_X       8
#define TILE_ENABLE_WRAP_Y       9
#define TILE_ENABLE_FLIP        10
#define TILE_SHIFT_DATA_OFFSET  11
#define TILE_PALETTE_START      12
#define TILE_PALETTE_END        15

// TILE_CTRL1
#define TILE_HSIZE_0             0
#define TILE_HSIZE_1             1
#define TILE_VSIZE_0             2
#define TILE_VSIZE_1             3
#define TILE_LAYER_HSIZE_0       4
#define TILE_LAYER_HSIZE_1       5
#define TILE_LAYER_VSIZE_0       6
#define TILE_LAYER_VSIZE_1       7

// TILE_DATA_OFFSET
#define TILE_INDEX_OFFSET_START  0
#define TILE_INDEX_OFFSET_END    7
#define TILE_IMAGE_OFFSET_START  8
#define TILE_IMAGE_OFFSET_END   15

// TILE_NOP_VALUE
#define TILE_NOP_VALUE_START     0
#define TILE_NOP_VALUE_END      12

// TILE_COLOR_KEY
#define TILE_TRANSP_VALUE_START  0
#define TILE_TRANSP_VALUE_END    7


// ==== Sprite register fields ====

// SPRITE_CTRL0
#define SPRITE_ENABLED             0
#define SPRITE_ENABLE_SCROLL       4
#define SPRITE_ENABLE_TRANSP       5
#define SPRITE_ENABLE_ALPHA        6
#define SPRITE_ENABLE_COLOR        7
#define SPRITE_FLIP_X              8
#define SPRITE_FLIP_Y              9
#define SPRITE_FLIP_XY            10
#define SPRITE_SHIFT_DATA_OFFSET  11

#define SPRITE_PALETTE_START      12
#define SPRITE_PALETTE_END        15
#define SPRITE_PALETTE_WIDTH  (SPRITE_PALETTE_END - SPRITE_PALETTE_START + 1)

// SPRITE_CTRL1
#define SPRITE_HSIZE_0             0
#define SPRITE_HSIZE_1             1
#define SPRITE_VSIZE_0             2
#define SPRITE_VSIZE_1             3
#define SPRITE_LAYER_HSIZE_0       4
#define SPRITE_LAYER_HSIZE_1       5
#define SPRITE_LAYER_VSIZE_0       6
#define SPRITE_LAYER_VSIZE_1       7

// SPRITE_REF_XY
#define SPRITE_REF_XY_X_START      0
#define SPRITE_REF_XY_X_END        5
#define SPRITE_REF_XY_X_WIDTH (SPRITE_REF_XY_X_END - SPRITE_REF_XY_X_START + 1)
#define SPRITE_REF_XY_Y_START      8
#define SPRITE_REF_XY_Y_END       13
#define SPRITE_REF_XY_Y_WIDTH (SPRITE_REF_XY_Y_END - SPRITE_REF_XY_Y_START + 1)

#endif  // __DUINOCUBE_CORE_DEFS_H__
