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

// A platform game demo.

#include <DuinoCube.h>
#include <SPI.h>

#if defined(__AVR_Atmega32U4__)
#include <Esplora.h>
#endif

#include "defines.h"
#include "map.h"
#include "printf.h"
#include "resources.h"
#include "sprites.h"

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

namespace {

// Store all sprites in one location.
Sprite sprites[MAX_NUM_SUBSPRITES + 1];

// Player sprite is a composite sprite.
CompositeSprite player;
Sprite* player_sprites = &sprites[0];

Sprite& bat = sprites[MAX_NUM_SUBSPRITES];

// Bat animation sequence.
const uint8_t kBatFrames[] = { 0, 1 };

// Initialize sprites.
void initSprites() {
  // Initialize player sprite.
  player.subsprites = player_sprites;
  player.rects = kChickSubFrames;

  player.x = 0;
  player.y = 0;
  player.w = CHICK_WIDTH;
  player.h = CHICK_HEIGHT;
  player.vx = 0;
  player.vy = 0;

  uint16_t subframe_offset = 0;
  for (int i = 0; i < MAX_NUM_SUBSPRITES; ++i) {
    Sprite& sprite = player.subsprites[i];
    sprite.state = SPRITE_ALIVE;
    sprite.dir = SPRITE_RIGHT;

    sprite.base_offset = g_player_offset + subframe_offset;

    const Rect& rect = player.rects[i];
    sprite.size = rect.w * rect.h;
    sprite.x = player.x + rect.x;
    sprite.y = player.y + rect.y;
    sprite.w = rect.w;
    sprite.h = rect.h;

    subframe_offset += sprite.size;
  }

  // Initialize a bat sprite.
  bat.state = SPRITE_ALIVE;
  bat.dir = SPRITE_RIGHT;
  bat.x = 0;
  bat.y = 0;
  bat.w = BAT_WIDTH;
  bat.h = BAT_HEIGHT;

  bat.base_offset = g_bat_offset;
  bat.size = BAT_SPRITE_SIZE;
}


// Handle player.
void updatePlayer() {
  // Read user input.
  // Handle directional pad input.
  GamepadState gamepad = DC.Gamepad.readGamepad();
  uint8_t dir_pad = 0;

  // TODO: Add this logic to DuinoCube library.
  if (gamepad.x == 0)
    dir_pad |= (1 << SPRITE_LEFT);
  else if (gamepad.x == UINT8_MAX)
    dir_pad |= (1 << SPRITE_RIGHT);

  if (gamepad.y == 0)
    dir_pad |= (1 << SPRITE_UP);
  else if (gamepad.y == UINT8_MAX)
    dir_pad |= (1 << SPRITE_DOWN);

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
    if (is_standing && (gamepad.buttons & (1 << GAMEPAD_BUTTON_1))) {
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

}  // namespace

void setup() {
  Serial.begin(115200);
  DC.begin();

  loadResources();
  setupLayers();

  initSprites();
  setupSprites(sprites, sizeof(sprites) / sizeof(sprites[0]));

  printf("Static data ends at 0x%04x (%u)\n", &__bss_end, &__bss_end);
  printf("Stack base at 0x%04x (%u)\n", &__stack, &__stack);

  // Initialize random generator with time.
  // TODO: Use a less deterministic seed value.
  srand(millis());
}

void loop() {
  // Wait for visible, non-vblanked region to do computations.
  while ((DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  updateSprite(&bat);
  animateSprite(&bat, kBatFrames, sizeof(kBatFrames) / sizeof(kBatFrames[0]),
                BAT_FRAME_PERIOD);

  updatePlayer();

  // Wait for Vblank to update rendering.
  while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  // Update sprite rendering.
  for (int i = 0; i < sizeof(sprites) / sizeof(sprites[0]); ++i) {
    const Sprite& sprite = sprites[i];

    // Update location.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprite.x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprite.y);

    // Update image.
    uint16_t ctrl0 = DC.Core.readWord(SPRITE_REG(i, SPRITE_CTRL_0));
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0),
                      (ctrl0 & ~SPRITE_FLIP_MASK) | sprite.flip);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_DATA_OFFSET), sprite.get_offset());
  }
}
