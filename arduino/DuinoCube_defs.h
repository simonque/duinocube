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

// DuinoCube memory and register definitions.

#ifndef __DUINOCUBE_DEFS_H__
#define __DUINOCUBE_DEFS_H__

// Main registers.
#define REG_ID             0x0000
#define REG_OUTPUT_STATUS  0x0002
#define REG_SCAN_X         0x0004
#define REG_SCAN_Y         0x0006
#define REG_SYS_CTRL       0x0008
#define REG_MEM_BANK       0x000a
#define REG_OUTPUT_CTRL    0x000c
#define REG_MODE_CTRL      0x000e

// SYS_CTRL register fields.
#define REG_SYS_CTRL_VRAM_ACCESS      0     // Enable MPU access to VRAM.

// Tile layer registers.
#define TILE_REG_BASE      0x0800
#define NUM_TILE_REGISTERS     16
#define TILE_REG_STRIDE       128  // Tile layer registers are spaced out in
                                   // 64-register blocks.
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
#define NUM_SPRITE_REGS        32  // Number of registers per sprite.
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
#define VRAM_BANK_BEGIN         2  // VRAM spans several 16-KB banks.
#define VRAM_BANK_END           4

#endif  // __DUINOCUBE_DEFS_H__
