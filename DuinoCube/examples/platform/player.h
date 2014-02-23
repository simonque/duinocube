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

// Contains player definitions.

#ifndef __PLAYER_H___
#define __PLAYER_H___

#include <stdint.h>

struct CompositeSprite;

// Player motion type.
enum {
  PLAYER_STANDING,
  PLAYER_FALLING,
  PLAYER_JUMPING,
  PLAYER_RUNNING,
  PLAYER_CROUCHING,
  NUM_PLAYER_MOTION_TYPES,
};

// Updates the player's location and orientation and apply gamepad input if
// necessary.
void updatePlayer(CompositeSprite* player_ptr, uint16_t dir_pad,
                  uint16_t buttons);

#endif  // __PLAYER_H___
