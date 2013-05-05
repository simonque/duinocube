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

// ChronoCube basic API.

#ifndef _CC_BASE_H_
#define _CC_BASE_H_

#include <stdint.h>

// These values represent sprite or tile dimensions in pixels.
enum CC_Dimension {
  CC_DIMENSION_8,
  CC_DIMENSION_16,
  CC_DIMENSION_32,
  CC_DIMENSION_64,
  CC_DIMENSION_MAX,
};

// Bit flags for flipping tiles or sprites.
enum CC_FlipFlags {
  CC_FLIP_X = (1 << 0),      // Flip horizontally.
  CC_FLIP_Y = (1 << 1),      // Flip vertically.
  CC_FLIP_XY = (1 << 2),     // Flip diagonally.
};

#endif  // _CC_BASE_H_
