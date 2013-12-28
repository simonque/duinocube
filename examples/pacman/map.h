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

#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>

#include "defines.h"

// A simple 2D vector.
struct Vector {
  int x, y;
};

// Convert sprite coordinates to tile grid coordinates.
uint8_t getTileX(uint16_t value);
uint8_t getTileY(uint16_t value);

// Get and set directional unit vectors.
const Vector& getDirVector(uint8_t dir);
void setDirVector(uint8_t dir, int x, int y);

// Returns true if the tile at the given grid coordinates is empty.
bool isEmptyTile(uint8_t x, uint8_t y);

// Returns true if the given tile grid coordinates is at an intersection. Two
// adjacent tiles must be empty.
bool isIntersection(uint8_t x, uint8_t y);

// Returns true if the given sprite is exactly at an intersection tile.
bool isAtIntersection(const Sprite& sprite);

// Returns the direction that is opposite to |dir|.
uint8_t getOppositeDir(uint8_t dir);

#endif  // __MAP_H__
