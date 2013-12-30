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

// Global definitions for Pacman.

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdint.h>

#define BG_TILEMAP_INDEX            0
#define DOTS_TILEMAP_INDEX          1
#define CLIPPING_TILEMAP_INDEX      3

#define BG_PALETTE_INDEX            0
#define SPRITE_PALETTE_INDEX        1

#define TILEMAP_WIDTH              32
#define TILEMAP_HEIGHT             32
#define TILEMAP_ENTRY_SIZE       sizeof(uint16_t)

// TODO: Add this to the DuinoCube library.
#define DEFAULT_EMPTY_TILE_VALUE     0x1fff

// Sprite definitions.
enum SpriteState {
  SPRITE_DEAD,
  SPRITE_ALIVE,
};

enum SpriteDirection {
  SPRITE_UP,
  SPRITE_DOWN,
  SPRITE_LEFT,
  SPRITE_RIGHT,
  NUM_SPRITE_DIRS,
};

#define NUM_GHOSTS            4

// Sprite dimensions in pixels.
#define SPRITE_WIDTH         16
#define SPRITE_HEIGHT        16
#define SPRITE_SIZE    (SPRITE_WIDTH * SPRITE_HEIGHT)

// Tile dimensions in pixels.
#define TILE_WIDTH            8
#define TILE_HEIGHT           8
#define TILE_SIZE      (TILE_WIDTH * TILE_HEIGHT)

// Offset between sprite top left corner and tilemap grid.
#define SPRITE_GRID_OFFSET_X  (TILE_WIDTH / 2)
#define SPRITE_GRID_OFFSET_Y  (TILE_HEIGHT / 2)

// Boundary wraparound thresholds.
#define WRAP_LEFT    (TILE_WIDTH - SPRITE_GRID_OFFSET_X)
#define WRAP_RIGHT   ((TILEMAP_WIDTH - 2) * TILE_WIDTH - SPRITE_GRID_OFFSET_X)
#define WRAP_TOP     (TILE_HEIGHT - SPRITE_GRID_OFFSET_Y)
#define WRAP_BOTTOM  ((TILEMAP_HEIGHT - 2) * TILE_HEIGHT - SPRITE_GRID_OFFSET_Y)

// Sprite start locations.
#define PLAYER_START_X       (15 * TILE_WIDTH + SPRITE_GRID_OFFSET_X)
#define PLAYER_START_Y       (22 * TILE_HEIGHT + SPRITE_GRID_OFFSET_Y)

// Ghost start locations.
#define GHOST_START_X0       (11 * TILE_WIDTH + SPRITE_GRID_OFFSET_X)
#define GHOST_START_DX       ( 2 * TILE_WIDTH)
#define GHOST_START_Y        (10 * TILE_HEIGHT + SPRITE_GRID_OFFSET_Y)

// Sprite data offset locations.
#define PLAYER_SPRITE_BASE_OFFSET          0
#define GHOST_SPRITE_BASE_OFFSET           (6 * SPRITE_SIZE)
#define NUM_FRAMES_PER_GHOST               6

// Transparent pixel value.
#define SPRITE_TRANSPRENT_VALUE            0

// Sprite size values to write to DuinoCube Core.
// TODO: Make these part of the DuinoCube library.
#define SPRITE_WIDTH_16       (1 << SPRITE_HSIZE_0)
#define SPRITE_HEIGHT_16      (1 << SPRITE_VSIZE_0)

// Sprite Z-depth. This sets it above all tile layers except for the clipping
// layer.
#define SPRITE_Z_DEPTH                   (CLIPPING_TILEMAP_INDEX - 1)

// Sprite data structure definition.
struct Sprite {
  uint8_t state;                // Alive or dead.
  uint8_t dir;                  // Direction sprite is facing.
  int16_t x, y;                 // Location in pixels.

  uint16_t base_offset;         // Base VRAM offset of sprite's frame images.
  uint8_t frame;                // Animation counter.
  uint8_t size;                 // Size of each sprite frame image in bytes.

  // Compute the sprite's current VRAM offset.
  inline uint16_t get_offset() const {
    return base_offset + frame * size;
  }
};

#endif  // __DEFINES_H__
