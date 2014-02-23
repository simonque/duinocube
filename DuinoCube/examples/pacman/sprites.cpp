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

#include "defines.h"
#include "map.h"

void updateSprite(Sprite* sprite_ptr, int speed) {
  Sprite& sprite = *sprite_ptr;

  const Vector& dir_vector = getDirVector(sprite.dir);
  sprite.x += dir_vector.x * speed;
  sprite.y += dir_vector.y * speed;

  // Handle wraparound.
  if (sprite.x < WRAP_LEFT)
    sprite.x = WRAP_RIGHT - 1;
  else if (sprite.x > WRAP_RIGHT)
    sprite.x = WRAP_LEFT + 1;

  if (sprite.y < WRAP_TOP)
    sprite.y = WRAP_BOTTOM - 1;
  else if (sprite.y > WRAP_BOTTOM)
    sprite.y = WRAP_TOP + 1;

  // Increment the animation counter.
  ++sprite.counter;
}
