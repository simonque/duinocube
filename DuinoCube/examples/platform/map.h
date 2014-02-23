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

// Map handling logic.

#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>

// Convert pixel coordinates to tile grid coordinates.
uint8_t getTileX(uint16_t value);
uint8_t getTileY(uint16_t value);

// Determine pixel offset relative to tile grid.
uint8_t getTileXOffset(uint16_t value);
uint8_t getTileYOffset(uint16_t value);

// Returns true if the tile at the given grid coordinates is empty.
bool isEmptyTile(uint16_t x, uint16_t y);

#endif  // __MAP_H__
