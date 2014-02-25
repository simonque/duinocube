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

#include <stdlib.h>

#include "defines.h"
#include "map.h"
#include "player.h"
#include "printf.h"
#include "resources.h"
#include "sprites.h"

// Generates a random value in the range [min, max).
#define RAND_RANGE(min, max) ((min) + rand() % (max - min))

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

namespace {

// Store all sprites in one location.
Sprite sprites[MAX_NUM_SUBSPRITES + NUM_BATS];

// Player sprite is a composite sprite.
CompositeSprite player;
Sprite* player_sprites = &sprites[0];

// Bats.
Sprite* bat_sprites = sprites + MAX_NUM_SUBSPRITES;
struct Bat {
  Sprite* sprite;
  int16_t vx, vy;     // Speed.
} bats[NUM_BATS];

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
  player.dir = SPRITE_RIGHT;
  player.motion = PLAYER_STANDING;
  player.frame_counter = 0;
  player.frame_index = 0;
  player.sprite_index = 0;

  uint16_t subframe_offset = 0;
  for (int i = 0; i < MAX_NUM_SUBSPRITES; ++i) {
    Sprite& sprite = player.subsprites[i];
    sprite.state = SPRITE_ALIVE;

    sprite.base_offset = g_player_offset + subframe_offset;

    const Rect& rect = player.rects[i];
    // The subsprite size is |rect.w * rect.h|. However, |size| is used by the
    // get_offset() function to find a frame. In this case, the stride between
    // each frame is the composite sprite size.
    sprite.size = player.w * player.h;
    sprite.w = rect.w;
    sprite.h = rect.h;

    subframe_offset += rect.w * rect.h;
  }
  updateCompositeSprite(&player);

  // Initialize bats.
  for (int i = 0; i < NUM_BATS; ++i) {
    Sprite& sprite = bat_sprites[i];

    // Spawn a bat somewhere.
    sprite.state = SPRITE_ALIVE;
    sprite.x = RAND_RANGE(BAT_SPAWN_X_MIN, BAT_SPAWN_X_MAX) * TILE_WIDTH;
    sprite.y = RAND_RANGE(BAT_SPAWN_Y_MIN, BAT_SPAWN_Y_MAX) * TILE_HEIGHT;
    sprite.w = BAT_WIDTH;
    sprite.h = BAT_HEIGHT;
    // Randomize their animation so they're not in sync.
    sprite.counter = RAND_RANGE(0, BAT_FRAME_PERIOD);

    // Determine its orientation.
    sprite.dir = (rand() % 2) ? SPRITE_RIGHT : SPRITE_LEFT;
    switch (sprite.dir) {
    default:
    case SPRITE_RIGHT:
      sprite.flip = 0;
      break;
    case SPRITE_LEFT:
      sprite.flip = (1 << SPRITE_FLIP_X);
      break;
    }

    sprite.base_offset = g_bat_offset;
    sprite.size = BAT_SPRITE_SIZE;

    // Initialize the bat object.
    Bat& bat = bats[i];
    bat.sprite = &sprite;
    // Generate a velocity for it.
    bat.vx = RAND_RANGE(BAT_SPAWN_VX_MIN, BAT_SPAWN_VX_MAX);
    if (sprite.dir == SPRITE_LEFT) {
      bat.vx *= -1;
    }
    bat.vy = RAND_RANGE(BAT_SPAWN_VY_MIN, BAT_SPAWN_VY_MAX);
    if (rand() % 2) {
      bat.vy *= -1;
    }
  }
}

// Read user input.
void readPlayerInput(uint16_t* dir_pad, uint16_t* buttons) {
  // Read user input.
  // Handle directional pad input.
  GamepadState gamepad = DC.Gamepad.readGamepad();
  *dir_pad = 0;

  // TODO: Add this logic to DuinoCube library.
  if (gamepad.x < 0)
    *dir_pad |= (1 << SPRITE_LEFT);
  else if (gamepad.x > 0)
    *dir_pad |= (1 << SPRITE_RIGHT);

  if (gamepad.y < 0)
    *dir_pad |= (1 << SPRITE_UP);
  else if (gamepad.y > 0)
    *dir_pad |= (1 << SPRITE_DOWN);

  *buttons = gamepad.buttons;
}

// If the level is larger than the tilemap, load the parts of the level that are
// farthest from the screen but still in the tilemap.
void loadLevelToTilemapEdges(uint16_t scroll_x, uint16_t scroll_y) {
  // Determine screen center based on scroll.
  int16_t center_tile_x = getTileX(scroll_x + SCREEN_WIDTH / 2);
  int16_t center_tile_y = getTileY(scroll_y + SCREEN_HEIGHT / 2);

  // Only do this for the X axis because the level is wider than the tilemap.
  // However, if the level were to become taller than the tilemap, it should be
  // likewise done for the Y axis.
  // if (LEVEL_WIDTH > TILEMAP_WIDTH)
  {
    // Load new data to the parts of the tilemap farthest from screen center.
    const uint16_t kOffsetsToLoad[] = {
      center_tile_x - TILEMAP_WIDTH / 2,
      center_tile_x + TILEMAP_WIDTH / 2 - 1,
    };
    // Access tilemap data.
    DC.Core.writeWord(REG_MEM_BANK, TILEMAP_BANK);
    uint16_t level_offset = 0;
    uint16_t tilemap_offset = 0;
    for (int i = 0; i < TILEMAP_HEIGHT; ++i) {
      uint16_t tile_value = DEFAULT_EMPTY_TILE_VALUE;
      for (int j = 0; j < ARRAY_SIZE(kOffsetsToLoad); ++j) {
        uint16_t offset = kOffsetsToLoad[j];
        DC.Mem.read(
            g_level_buffer + level_offset + offset * sizeof(tile_value),
            &tile_value, sizeof(tile_value));
        DC.Core.writeWord(
            TILEMAP(LEVEL_TILEMAP_INDEX) + tilemap_offset +
                (offset % TILEMAP_WIDTH) * sizeof(tile_value),
            tile_value);
      }
      tilemap_offset += TILEMAP_WIDTH * sizeof(tile_value);
      level_offset += LEVEL_WIDTH * sizeof(tile_value);
    }
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  DC.begin();

  loadResources();
  setupLayers();

  initSprites();
  setupSprites(sprites, ARRAY_SIZE(sprites));

  printf("Static data ends at 0x%04x (%u)\n", &__bss_end, &__bss_end);
  printf("Stack base at 0x%04x (%u)\n", &__stack, &__stack);

  // Initialize random generator with time.
  // TODO: Use a less deterministic seed value.
  srand(millis());
}

// Camera scroll params.
static uint16_t scroll_x = 0;
static uint16_t scroll_y = 0;

void loop() {
  // Wait for visible, non-vblanked region to do computations.
  while ((DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  for (int i = 0; i < NUM_BATS; ++i) {
    Bat& bat = bats[i];
    // Move the bat.
    bat.sprite->x += bat.vx;
    bat.sprite->y += bat.vy;

    // Update the bat sprite.
    updateSprite(bat.sprite);
    animateSprite(bat.sprite, kBatFrames, ARRAY_SIZE(kBatFrames),
                  BAT_FRAME_PERIOD);
  }

  uint16_t dir_pad, buttons;
  readPlayerInput(&dir_pad, &buttons);
  updatePlayer(&player, dir_pad, buttons);

  // Determine new scroll location based on player location. Limit scrolling to
  // the edges of the level.
  int16_t center_x = player.x + player.w / 2;
  int16_t center_y = player.y + player.h / 2;

  // Save old scroll location.
  uint16_t old_scroll_x = scroll_x;
  uint16_t old_scroll_y = scroll_y;

  // X scrolling.
  if (center_x < SCREEN_WIDTH / 2) {
    // Limit on left edge.
    scroll_x = 0;
  } else if (center_x + SCREEN_WIDTH / 2 >= LEVEL_PIXEL_WIDTH) {
    // Limit on right edge.
    scroll_x = LEVEL_PIXEL_WIDTH - SCREEN_WIDTH;
  } else {
    // Scroll when in the center part.
    scroll_x = center_x - SCREEN_WIDTH / 2;
  }
  // Y scrolling.
  if (center_y < SCREEN_HEIGHT / 2) {
    // Limit on top edge.
    scroll_y = 0;
  } else if (center_y + SCREEN_HEIGHT / 2 >= LEVEL_PIXEL_HEIGHT) {
    // Limit on bottom edge.
    scroll_y = LEVEL_PIXEL_HEIGHT - SCREEN_HEIGHT;
  } else {
    // Scroll when in the center part.
    scroll_y = center_y - SCREEN_HEIGHT / 2;
  }

  // Load new parts of level into the tilemap.
  loadLevelToTilemapEdges(scroll_x, scroll_y);

  // Wait for Vblank to update rendering.
  while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  // Scroll camera.
  DC.Core.writeWord(REG_SCROLL_X, scroll_x);
  DC.Core.writeWord(REG_SCROLL_Y, scroll_y);

  // Scroll the background and moon at slower rates relative to the screen..
  DC.Core.writeWord(TILE_LAYER_REG(MOON_TILEMAP_INDEX, TILE_OFFSET_X),
                    scroll_x / MOON_X_SCROLL_FACTOR);
  DC.Core.writeWord(TILE_LAYER_REG(MOON_TILEMAP_INDEX, TILE_OFFSET_Y),
                    scroll_y / MOON_Y_SCROLL_FACTOR);

  DC.Core.writeWord(TILE_LAYER_REG(BG_TILEMAP_INDEX, TILE_OFFSET_X),
                    scroll_x / BG_X_SCROLL_FACTOR);
  DC.Core.writeWord(TILE_LAYER_REG(BG_TILEMAP_INDEX, TILE_OFFSET_Y),
                    scroll_y / BG_Y_SCROLL_FACTOR);

  // Update sprite rendering.
  for (int i = 0; i < ARRAY_SIZE(sprites); ++i) {
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
