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

// Sprite functions.

#include "sprites.h"

#include <DuinoCube.h>

#include "defines.h"

void updateSprite(Sprite* sprite_ptr) {
  Sprite& sprite = *sprite_ptr;

  // TODO: update sprite location.

  // Increment the animation counter.
  ++sprite.counter;
}

void setupSprites(const Sprite* sprites, int num_sprites) {
  // Set sprite Z-depth.
  DC.Core.writeWord(REG_SPRITE_Z, SPRITE_Z_DEPTH);

  // Set up sprite rendering.
  for (int i = 0; i < num_sprites; ++i) {
    const Sprite& sprite = sprites[i];

    // Set sprite size.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_1),
                      SPRITE_WIDTH_32 | SPRITE_HEIGHT_32);

    // Set image data offset.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_DATA_OFFSET), sprite.get_offset());

    // Set transparency.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_COLOR_KEY), DEFAULT_COLOR_KEY);

    // Set location.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprite.x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprite.y);

    // Enable the sprite.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0),
                      (1 << SPRITE_ENABLED) |
                      (1 << SPRITE_ENABLE_TRANSP) |
                      (1 << SPRITE_ENABLE_SCROLL) |
                      (1 << SPRITE_SHIFT_DATA_OFFSET) |
                      (SPRITE_PALETTE_INDEX << SPRITE_PALETTE_START));
  }
}

#include <stdio.h>

void animateSprite(Sprite* sprite_ptr, const uint8_t* frames,
                   uint8_t num_frames, uint8_t frame_period) {
  Sprite& sprite = *sprite_ptr;

  // Compute current frame based on the counter.
  uint8_t frame_index = (sprite.counter / frame_period) % num_frames;
  sprite.counter %= (frame_period * num_frames);

  // Update to a new frame.
  sprite.frame = frames[frame_index];
}
