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

#include "map.h"

#include <DuinoCube.h>

#include "defines.h"
#include "resources.h"

bool isEmptyTile(uint16_t x, uint16_t y) {
  uint16_t tile_value = 0;
  uint16_t offset = sizeof(tile_value) * (x + y * LEVEL_WIDTH);
  DC.Sys.readSharedRAM(g_walk_buffer + offset, &tile_value, sizeof(tile_value));

  return (tile_value == DEFAULT_EMPTY_TILE_VALUE);
}
