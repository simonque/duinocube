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

// Contains sprite definitions.

#ifndef __SPRITES_H__
#define __SPRITES_H__

#include <stdint.h>

// Sprite data structure definition.
struct Sprite {
  uint8_t state;                // Alive or dead.
  uint8_t dir;                  // Direction sprite is facing.
  int16_t x, y;                 // Location in pixels.

  uint16_t base_offset;         // Base VRAM offset of sprite's frame images.
  uint16_t size;                // Size of each sprite frame image in bytes.
  uint8_t frame;                // Current frame image.
  uint16_t flip;                // Current frame flip flags.
  uint16_t counter;             // Animation counter.

  Sprite() : frame(0),
             flip(0),
             counter(0) {}

  // Compute the sprite's current VRAM offset.
  inline uint16_t get_offset() const {
    return base_offset + frame * size;
  }
};

// Move and animate sprite.
void updateSprite(Sprite* sprite_ptr, int speed);

#endif  // __SPRITES_H__
