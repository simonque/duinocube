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

// Global definitions for platform game.

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdint.h>

enum {
  BG_TILEMAP_INDEX,
  MOON_TILEMAP_INDEX,
  LEVEL_TILEMAP_INDEX,
};

enum {
  BG_PALETTE_INDEX,
};

// TODO: Add this to the DuinoCube library.
#define DEFAULT_EMPTY_TILE_VALUE     0x1fff
#define DEFAULT_COLOR_KEY              0x00

// Tile dimensions in pixels.
#define TILE_WIDTH            8
#define TILE_HEIGHT           8
#define TILE_SIZE      (TILE_WIDTH * TILE_HEIGHT)

// Map dimensions in tiles.
#define TILEMAP_WIDTH              32
#define TILEMAP_HEIGHT             32
#define TILEMAP_ENTRY_SIZE       sizeof(uint16_t)

// Map dimensions in pixels.
#define MAP_WIDTH           (TILEMAP_WIDTH * TILE_WIDTH)
#define MAP_HEIGHT          (TILEMAP_WIDTH * TILE_HEIGHT)

// Screen dimensions.
// TODO: these should be read from DuinoCube registers.
#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240

// Level dimensions in tiles.  This could be different from the size of the
// tilemap.
#define LEVEL_WIDTH          64
#define LEVEL_HEIGHT         32
#define LEVEL_SIZE           (LEVEL_WIDTH * LEVEL_HEIGHT * TILEMAP_ENTRY_SIZE)

#endif  // __DEFINES_H__
