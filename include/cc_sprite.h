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

// ChronoCube sprite API.

#ifndef _CC_SPRITE_H_
#define _CC_SPRITE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sprite register access.
void CC_Sprite_SetRegister(uint16_t index, uint8_t reg, uint16_t value);
uint16_t CC_Sprite_GetRegister(uint16_t index, uint8_t reg);

// Set the location of the sprite in world space.
void CC_Sprite_SetLocation(uint16_t index, uint16_t x, uint16_t y);

// Set the reference point on the sprite to be used in rendering operations.
void CC_Sprite_SetRefLocation(uint16_t index, uint8_t x, uint8_t y);

// Set the dimensions of the sprite, in pixels.
// |dim_x| and |dim_y| are not actual dimension values.  They are enums for the
// values defined in CC_Sprite_Dimensions.
// If invalid enum values are given, this function will do nothing.
void CC_Sprite_SetDimensions(uint16_t index, uint8_t dim_x, uint8_t dim_y);

// Enable transparent color key for sprite.
void CC_Sprite_EnableTransparency(uint16_t index, uint8_t enabled);

// Specify transparent color key for sprite.
void CC_Sprite_SetTransparentValue(uint16_t index, uint8_t value);

// Enable alpha blending for sprite.
void CC_Sprite_EnableAlpha(uint16_t index, uint8_t enabled);

// Set alpha value of sprite.
void CC_Sprite_SetAlpha(uint16_t index, uint8_t alpha);

// Enable or disable a sprite.
void CC_Sprite_SetEnabled(uint16_t index, uint8_t enabled);

// Select a palette to be used by the sprite.
void CC_Sprite_SetPalette(uint16_t index, uint8_t palette_index);

// Specify the offset in VRAM of this sprite's data, given as a multiple of
// 64-byte blocks.
void CC_Sprite_SetDataOffset(uint16_t index, uint16_t data_offset);

// Specify horizontal, vertical, and diagonal flipping.
void CC_Sprite_SetFlip(uint16_t index, uint8_t flip_flags);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // _CC_SPRITE_H_
