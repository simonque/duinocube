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

// ChronoCube tile layer register definitions.

#ifndef _TILE_REGISTERS_H_
#define _TILE_REGISTERS_H_

#define NUM_TILE_LAYERS             4

// Number of tile registers for each layer.
#define NUM_TILE_REGISTERS         16

// Register offsets within each register block.
#define TILE_CTRL0               0x00
#define TILE_CTRL1               0x01
#define TILE_DATA_OFFSET         0x03

#define TILE_NOP_VALUE           0x04
#define TILE_COLOR_KEY           0x05

#define TILE_OFFSET_X            0x08
#define TILE_OFFSET_Y            0x09

#define TILE_ROT_ANGLE           0x08
#define TILE_ROT_X               0x0a
#define TILE_ROT_Y               0x0b

#define TILE_SCALE_X             0x0c
#define TILE_SCALE_Y             0x0d

// Register fields

// TILE_CTRL0
#define TILE_LAYER_ENABLED       0
#define TILE_ENABLE_8_BIT        2
#define TILE_ENABLE_NOP          3
#define TILE_ENABLE_SCROLL       4
#define TILE_ENABLE_TRANSP       5
#define TILE_ENABLE_ALPHA        6
#define TILE_ENABLE_COLOR        7
#define TILE_ENABLE_WRAP_X       8
#define TILE_ENABLE_WRAP_Y       9
#define TILE_ENABLE_FLIP        10
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

// TILE_TRANSP_VALUE
#define TILE_TRANSP_VALUE_START  0
#define TILE_TRANSP_VALUE_END    7


// Tile register address definitions.
#define TILE_REG_ADDR_BASE     0x0800
#define TILE_REG_ADDR_STEP       0x80

#endif  // _TILE_REGISTERS_H_
