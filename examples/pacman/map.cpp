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

// Pacman map handling logic.

#include "map.h"

#include <stdio.h>

#include <DuinoCube.h>

#include "sprites.h"

// Generic sprite coordinate to tile grid coordinate conversion formula.
#define SPRITE_TO_TILE(value, tile_size, offset) \
  (((value) + (offset)) / (tile_size))

// Computes the amount of offset between a sprite and the tile grid.
#define SPRITE_TO_TILE_OFFSET(value, tile_size, offset) \
  (((value) + (offset)) % (tile_size))

static Vector g_directions[NUM_SPRITE_DIRS];

uint8_t getTileX(uint16_t value) {
  return SPRITE_TO_TILE(value, TILE_WIDTH, -SPRITE_GRID_OFFSET_X);
}

uint8_t getTileY(uint16_t value) {
  return SPRITE_TO_TILE(value, TILE_HEIGHT, -SPRITE_GRID_OFFSET_Y);
}

const Vector& getDirVector(uint8_t dir) {
  if (dir < NUM_SPRITE_DIRS)
    return g_directions[dir];

  printf("Invalid direction enum: %d\n", dir);
  return g_directions[0];
}

void setDirVector(uint8_t dir, int x, int y) {
  if (dir < NUM_SPRITE_DIRS) {
    g_directions[dir].x = x;
    g_directions[dir].y = y;
  } else {
    printf("Invalid direction enum: %d\n", dir);
  }
}

bool isEmptyTile(uint8_t x, uint8_t y) {
  // Look up tile info stored in DuinoCube Core's tile map.
  uint16_t offset = ((uint16_t)TILEMAP_WIDTH * y + x) * TILEMAP_ENTRY_SIZE;

  DC.Core.writeWord(REG_MEM_BANK, TILEMAP_BANK);
  uint16_t value = DC.Core.readWord(TILEMAP(BG_TILEMAP_INDEX) + offset);
  DC.Core.writeWord(REG_MEM_BANK, 0);
  return value == DEFAULT_EMPTY_TILE_VALUE;
}

bool isIntersection(uint8_t x, uint8_t y) {
  // Intersections can only be on empty tiles.
  if (!isEmptyTile(x, y))
    return false;

  // Count the number of adjacent empty tiles.
  uint8_t num_adjacent_empty_tiles = 0;
  uint8_t empty_tile_dir_mask = 0;
  for (uint8_t dir = 0; dir < NUM_SPRITE_DIRS; ++dir) {
    Vector dir_vector = getDirVector(dir);
    if (isEmptyTile(x + dir_vector.x, y + dir_vector.y)) {
      ++num_adjacent_empty_tiles;
      empty_tile_dir_mask |= (1 << dir);
    }
  }

  // Three and four are clearly intersections.
  // TODO: Need to check the corners too.
  if (num_adjacent_empty_tiles == 3 ||
      num_adjacent_empty_tiles == 4) {
    return true;
  }

  // Two is an intersection if it's a corner (non-straight).
  if (num_adjacent_empty_tiles == 2 &&
      empty_tile_dir_mask != ((1 << SPRITE_UP) | (1 << SPRITE_DOWN)) &&
      empty_tile_dir_mask != ((1 << SPRITE_LEFT) | (1 << SPRITE_RIGHT))) {
    return true;
  }

  return false;
}

bool isAtIntersection(const Sprite& sprite) {
  // If the sprite is not exactly on a tile, it's not at an intersection.
  if (!isAlignedToTileGrid(sprite)) {
    return false;
  }
  // Otherwise, compute based on the tile grid coordinates.
  return isIntersection(getTileX(sprite.x), getTileY(sprite.y));
}

bool isAlignedToTileGrid(const Sprite& sprite) {
  return (SPRITE_TO_TILE_OFFSET(sprite.x, TILE_WIDTH,
                                -SPRITE_GRID_OFFSET_X) == 0) &&
         (SPRITE_TO_TILE_OFFSET(sprite.y, TILE_HEIGHT,
                                -SPRITE_GRID_OFFSET_Y) == 0);
}

uint8_t getOppositeDir(uint8_t dir) {
  switch (dir) {
  case SPRITE_UP:
    return SPRITE_DOWN;
  case SPRITE_DOWN:
    return SPRITE_UP;
  case SPRITE_LEFT:
    return SPRITE_RIGHT;
  case SPRITE_RIGHT:
    return SPRITE_LEFT;
  default:
    return dir;
  }
}
