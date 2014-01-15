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

#include "defines.h"
#include "map.h"
#include "sprites.h"

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

  // Update component sprites.
  updateCompositeSprite(&player);
}
