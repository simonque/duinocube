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

// ChronoCube emulator layer code.

#include "cc_tile_layer.h"

#include <string.h>

#include "cc_internal.h"

// Wrapper define for layer lookup by index.
#define LAYER CC_GetTileLayer(index)

void CC_TileLayer_SetData(uint8_t index, void* data, uint32_t size) {
  memcpy(LAYER->tiles, data, size);
}

void CC_TileLayer_SetDataAt(uint8_t index,
                           uint16_t value,
                           uint32_t x,
                           uint32_t y) {
  LAYER->tiles[x + LAYER->w * y] = value;
}

void CC_TileLayer_SetOffset(uint8_t index, uint16_t x, uint16_t y) {
  LAYER->x = x;
  LAYER->y = y;
}

void CC_TileLayer_SetEnabled(uint8_t index, uint8_t enabled) {
  LAYER->enabled = enabled;
}

void CC_TileLayer_EnableNopTile(uint8_t index, uint8_t enabled) {
  LAYER->enable_nop = enabled;
}

void CC_TileLayer_SetNopValue(uint8_t index, uint16_t nop_value) {
  LAYER->nop_value = nop_value;
}

void CC_TileLayer_EnableTransparency(uint8_t index, uint8_t enabled) {
  LAYER->enable_trans = enabled;
}

void CC_TileLayer_SetTransparentValue(uint8_t index, uint8_t value) {
  LAYER->trans_value = value;
}

void CC_TileLayer_EnableAlpha(uint8_t index, uint8_t enabled) {
  LAYER->enable_alpha = enabled;
}

void CC_TileLayer_SetAlpha(uint8_t index, uint8_t alpha) {
  LAYER->alpha = alpha;
}

void CC_TileLayer_SetPalette(uint8_t index, uint8_t palette_index) {
  LAYER->palette = palette_index;
}
