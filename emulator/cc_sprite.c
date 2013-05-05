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

// ChronoCube emulator sprite code.

#include "cc_sprite.h"

#include <assert.h>

#include "cc_base.h"
#include "cc_internal.h"

// Wrapper define for sprite lookup by index.
#define SPRITE CC_GetSprite(index)

static const uint8_t kDimensionValues[] = { 8, 16, 32, 64 };

// Size in bytes of a 8x8 data block.
static const int kDataBlockSize = 64;

void CC_Sprite_SetLocation(uint16_t index, uint16_t x, uint16_t y) {
  SPRITE->x = x;
  SPRITE->y = y;
}

void CC_Sprite_SetReferenceLocation(uint16_t index, uint8_t x, uint8_t y) {
  SPRITE->ref_x = x;
  SPRITE->ref_y = y;
}

void CC_Sprite_SetDimensions(uint16_t index, uint8_t dim_x, uint8_t dim_y) {
  if (dim_x >= CC_DIMENSION_MAX || dim_y >= CC_DIMENSION_MAX)
    return;
  SPRITE->w = kDimensionValues[dim_x];
  SPRITE->h = kDimensionValues[dim_y];
}

void CC_Sprite_EnableTransparency(uint16_t index, uint8_t enabled) {
  SPRITE->enable_trans = enabled;
}

void CC_Sprite_SetTransparentValue(uint16_t index, uint8_t value) {
  SPRITE->trans_value = value;
}

void CC_Sprite_EnableAlpha(uint16_t index, uint8_t enabled) {
  SPRITE->enable_alpha = enabled;
}

void CC_Sprite_SetAlpha(uint16_t index, uint8_t alpha) {
  SPRITE->alpha = alpha;
}

void CC_Sprite_SetEnabled(uint16_t index, uint8_t enabled) {
  SPRITE->enabled = enabled;
}

void CC_Sprite_SetPalette(uint16_t index, uint8_t palette_index) {
  SPRITE->palette = palette_index;
}

void CC_Sprite_SetDataOffset(uint16_t index, uint16_t data_offset) {
  SPRITE->data_offset = data_offset * kDataBlockSize;
}

void CC_Sprite_SetFlip(uint16_t index, uint8_t flip_flags) {
  SPRITE->flip_x = flip_flags & CC_FLIP_X;
  SPRITE->flip_y = flip_flags & CC_FLIP_Y;
  SPRITE->flip_xy = flip_flags & CC_FLIP_XY;
}
