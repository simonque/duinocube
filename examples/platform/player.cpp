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

// Player control functions.

#include "player.h"

#include <avr/pgmspace.h>

#include "defines.h"
#include "map.h"
#include "sprites.h"

// Player frame indexes. See the player sprite image data.
enum {
  FRAME_STANDING_0,
  FRAME_CROUCHING_0,
  FRAME_CROUCHING_1,
  FRAME_CROUCHING_2,
  FRAME_CROUCHING_3,
  FRAME_JUMPING_0,
  FRAME_JUMPING_1,
  FRAME_JUMPING_2,
  FRAME_JUMPING_3,
  FRAME_RUNNING_0,
  FRAME_RUNNING_1,
  FRAME_RUNNING_2,
  FRAME_RUNNING_3,
  FRAME_RUNNING_4,
  FRAME_RUNNING_5,
  FRAME_RUNNING_6,
  FRAME_RUNNING_7,
};

// Animation sequences for various motions.
const uint8_t kPlayerStandingFrames[] PROGMEM = {
  FRAME_STANDING_0,
};
const uint8_t kPlayerRunningFrames[] PROGMEM = {
  FRAME_RUNNING_0,
  FRAME_RUNNING_1,
  FRAME_RUNNING_2,
  FRAME_RUNNING_3,
  FRAME_RUNNING_4,
  FRAME_RUNNING_5,
  FRAME_RUNNING_6,
  FRAME_RUNNING_7,
};
const uint8_t kPlayerCrouchingFrames[] PROGMEM = {
  FRAME_CROUCHING_0,
  FRAME_CROUCHING_1,
  FRAME_CROUCHING_2,
  FRAME_CROUCHING_3,
};
const uint8_t kPlayerJumpingFrames[] PROGMEM = {
  FRAME_JUMPING_0,
  FRAME_JUMPING_1,
  FRAME_JUMPING_2,
  FRAME_JUMPING_3,
};
const uint8_t kPlayerFallingFrames[] PROGMEM = {
  FRAME_JUMPING_0,
  FRAME_JUMPING_1,
  FRAME_JUMPING_2,
  FRAME_JUMPING_3,
};

namespace {

// These should match the order of the player motion enums.
const uint8_t* kAnimations[] = {
  kPlayerStandingFrames,
  kPlayerFallingFrames,
  kPlayerJumpingFrames,
  kPlayerRunningFrames,
  kPlayerCrouchingFrames,
};

const uint8_t kAnimationLengths[] = {
  ARRAY_SIZE(kPlayerStandingFrames),
  ARRAY_SIZE(kPlayerFallingFrames),
  ARRAY_SIZE(kPlayerRunningFrames),
  ARRAY_SIZE(kPlayerJumpingFrames),
  ARRAY_SIZE(kPlayerCrouchingFrames),
};

void updatePlayerAnimation(CompositeSprite* player_ptr) {
  CompositeSprite& player = *player_ptr;

  // Update the frame counter. Update the frame if the counter has reached the
  // limit for going to the next frame.
  if (++player.frame_counter < PLAYER_FRAME_PERIOD) {
    return;
  }

  // Reset frame counter.
  player.frame_counter = 0;

  // Update the animation frame index.
  bool loop_animation = false;
  uint8_t motion = player.motion;
  switch (motion) {
  case PLAYER_RUNNING:
    loop_animation = true;
    break;
  case PLAYER_STANDING:
  case PLAYER_FALLING:
  case PLAYER_JUMPING:
  case PLAYER_CROUCHING:
  default:
    loop_animation = false;
    break;
  }

  ++player.frame_index;

  // The animation should either loop around or terminate.
  uint8_t curr_anim_length = kAnimationLengths[motion];
  if (loop_animation && player.frame_index >= curr_anim_length) {
    player.frame_index = 0;
  } else if (!loop_animation && player.frame_index >= curr_anim_length) {
    player.frame_index = curr_anim_length - 1;
  }

  // Update the sprite image index.
  player.sprite_index = pgm_read_byte(kAnimations[motion] + player.frame_index);
}

}  // namespace

#include <stdio.h>

void updatePlayer(CompositeSprite* player_ptr, uint16_t dir_pad,
                  uint16_t buttons) {
  CompositeSprite& player = *player_ptr;

  // Apply acceleration.
  if (dir_pad & (1 << SPRITE_LEFT)) {
    player.vx -= PLAYER_ACCEL_X;
    if (player.vx < -PLAYER_MAX_VX);
      player.vx = -PLAYER_MAX_VX;
  } else if (dir_pad & (1 << SPRITE_RIGHT)) {
    player.vx += PLAYER_ACCEL_X;
    if (player.vx > PLAYER_MAX_VX);
      player.vx = PLAYER_MAX_VX;
  } else {  // Decelerate when the directional pad is neutral along this axis.
    if (player.vx > 0) {
      player.vx -= PLAYER_ACCEL_X;
      if (player.vx < 0)
        player.vx = 0;
    } else if (player.vx < 0) {
      player.vx += PLAYER_ACCEL_X;
      if (player.vx > 0)
        player.vx = 0;
    }
  }
  if (player.vx > 0) {
    player.dir = SPRITE_RIGHT;
  } else if (player.vx < 0) {
    player.dir = SPRITE_LEFT;
  }

  if (dir_pad & (1 << SPRITE_DOWN)) {
    // TODO: crouch.
  }

  // Test for collision with level.
  // Assume that each displacement is smaller than the size of the level tiles.
  if (player.vx) {
    bool blocked_x = false;
    // Determine which edge is leading the motion.
    uint16_t test_x_edge = player.x + ((player.vx > 0) ? (player.w - 1) : 0);
    uint16_t tile_x = getTileX(test_x_edge + player.vx);
    uint16_t tile_y0 = getTileY(player.y);
    uint16_t tile_y1 = getTileY(player.y + player.h - 1);
    for (uint16_t tile_y = tile_y0; tile_y <= tile_y1 && !blocked_x; ++tile_y) {
      // Check if each tile along the edge is blocked.
      if (!isEmptyTile(tile_x, tile_y)) {
        blocked_x = true;
      }
    }
    if (!blocked_x) {
      // If not blocked, move player normally.
      player.x += player.vx;
    } else {
      // If blocked, move to be one short of the destination.
      uint8_t x_offset = getTileXOffset(player.x);
      if (x_offset == 0) {
        if (player.vx < 0) {
          // Do nothing to move.
        } else {  // if (player.vx > 0)
          // Do nothing to move.
        }
      } else {
        if (player.vx < 0) {
          player.x -= x_offset;
        } else {  // if (player.vx > 0)
          player.x += (TILE_WIDTH - x_offset);
        }
      }
      player.vx = 0;
    }
  }

  bool is_standing = false;
  if (player.vy) {
    bool blocked_y = false;
    // Determine which edge is leading the motion.
    uint16_t test_y_edge = player.y + ((player.vy > 0) ? (player.h - 1) : 0);
    uint16_t tile_y = getTileY(test_y_edge + player.vy);
    uint16_t tile_x0 = getTileY(player.x);
    uint16_t tile_x1 = getTileY(player.x + player.w - 1);
    for (uint16_t tile_x = tile_x0; tile_x <= tile_x1 && !blocked_y; ++tile_x) {
      // Check if each tile along the edge is blocked.
      if (!isEmptyTile(tile_x, tile_y)) {
        blocked_y = true;
      }
    }
    if (!blocked_y) {
      // If not blocked, move player normally.
      player.y += player.vy;
    } else {
      // If blocked, move to be one short of the destination.
      uint8_t y_offset = getTileYOffset(player.y);
      if (y_offset == 0) {
        if (player.vy < 0) {
          // Do nothing to move.
        } else {  // if (player.vy > 0)
          // Do nothing to move.
        }
      } else {
        if (player.vy < 0) {
          player.y -= y_offset;
        } else {  // if (player.vx > 0)
          player.y += (TILE_HEIGHT - y_offset);
        }
      }
      is_standing = (player.vy > 0);
      player.vy = 0;
    }
  } else {
    // Check if player is standing on something.
    uint16_t tile_y = getTileY(player.y + player.h);
    uint16_t tile_x0 = getTileY(player.x);
    uint16_t tile_x1 = getTileY(player.x + player.w - 1);
    for (uint16_t tile_x = tile_x0;
         tile_x <= tile_x1 && !is_standing;
         ++tile_x) {
      // Check if each tile along the edge is blocked.
      if (!isEmptyTile(tile_x, tile_y)) {
        is_standing = true;
      }
    }
    // Allow for jump.
    if (is_standing && (buttons & (1 << GAMEPAD_BUTTON_1))) {
      player.vy -= PLAYER_JUMP_ACCEL;
    }
  }

  // Apply gravity.
  if (!is_standing) {
    player.vy += PLAYER_GRAVITY;
    if (player.vy > PLAYER_MAX_VY)
      player.vy = PLAYER_MAX_VY;
  }

  // Determine current player motion.
  uint8_t prev_motion = player.motion;
  if (is_standing) {
    if (player.vx != 0) {
      player.motion = PLAYER_RUNNING;
    } else {
      player.motion = PLAYER_STANDING;
    }
  } else {
    if (player.vy <= 0) {
      player.motion = PLAYER_JUMPING;
    } else {
      player.motion = PLAYER_FALLING;
    }
  }
  // If the player's motion changed, reset the animation sequence.
  if (prev_motion != player.motion) {
    player.frame_counter = 0;
    player.frame_index = 0;
    printf("%d -> %d\n", prev_motion, player.motion);
  }

  // Animate.
  updatePlayerAnimation(&player);

  // Update component sprites.
  updateCompositeSprite(&player);
}
