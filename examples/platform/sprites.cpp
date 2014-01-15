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

namespace {

  // Values of the sprite dimension fields in the sprite control register.
  enum {
    SPRITE_DIMENSION_8,
    SPRITE_DIMENSION_16,
    SPRITE_DIMENSION_32,
    SPRITE_DIMENSION_64,
  };

  // Gets the sprite dimension code for a given dimension that is one of:
  // 8, 16, 32, 64.
  // If it doesn't match any of these, returns the code for 8.
  uint8_t getSpriteDimension(uint8_t size) {
    switch(size) {
    case 64:
      return SPRITE_DIMENSION_64;
    case 32:
      return SPRITE_DIMENSION_32;
    case 16:
      return SPRITE_DIMENSION_16;
    case 8:
    default:
      return SPRITE_DIMENSION_8;
    }
  }

}  // namespace

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
                      (getSpriteDimension(sprite.w) << SPRITE_HSIZE_0) |
                      (getSpriteDimension(sprite.h) << SPRITE_VSIZE_0));

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

void animateSprite(Sprite* sprite_ptr, const uint8_t* frames,
                   uint8_t num_frames, uint8_t frame_period) {
  Sprite& sprite = *sprite_ptr;

  // Compute current frame based on the counter.
  uint8_t frame_index = (sprite.counter / frame_period) % num_frames;
  sprite.counter %= (frame_period * num_frames);

  // Update to a new frame.
  sprite.frame = frames[frame_index];
}

void updateCompositeSprite(CompositeSprite* sprite) {
  for (int i = 0; i < MAX_NUM_SUBSPRITES; ++i) {
    Sprite& sub_sprite = sprite->subsprites[i];
    const Rect& rect = sprite->rects[i];

    // Update subsprite location and orientation based on composite sprite
    // location and orientation.
    switch (sprite->dir) {
    default:
    case SPRITE_RIGHT:
      sub_sprite.x = sprite->x + rect.x;
      sub_sprite.flip = 0;
      break;
    case SPRITE_LEFT:
      sub_sprite.x = sprite->x + sprite->w - rect.x - rect.w;
      sub_sprite.flip = (1 << SPRITE_FLIP_X);
      break;
    }
    sub_sprite.y = sprite->y + rect.y;
    sub_sprite.dir = sprite->dir;
    sub_sprite.frame = sprite->sprite_index;
  }
}
