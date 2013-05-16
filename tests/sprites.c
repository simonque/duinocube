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

// Many sprites drawing demo.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc_core.h"
#include "cc_sprite.h"
#include "system.h"

#include "registers.h"
#include "sprite_registers.h"

#include "data/sprites16.raw.h"
#include "data/sprites32.raw.h"
#include "data/sprites.pal.h"

#define NUM_PALETTE_ENTRIES      256

static void setup_palette(void) {
  uint32_t value;
  for (uint16_t i = 0; i < NUM_PALETTE_ENTRIES; ++i) {
    value = pgm_read_dword(&sprites_pal_data32[i]);
    CC_SetPaletteData(&value, 0, i * sizeof(value), sizeof(value));\
  }
}

static uint16_t sprites16_offset = 0;
static uint16_t sprites32_offset = 0;

static void setup_vram(void) {
  uint16_t offset = 0;

  for (int i = 0; i < SPRITES16_BMP_RAW_DATA_SIZE / sizeof(uint32_t); ++i) {
    uint32_t value = pgm_read_dword(&sprites16_bmp_raw_data32[i]);
    CC_SetVRAMData(&value, offset, sizeof(value));
    offset += sizeof(value);
  }
  sprites16_offset = 0;

  for (int i = 0; i < SPRITES32_BMP_RAW_DATA_SIZE / sizeof(uint32_t); ++i) {
    uint32_t value = pgm_read_dword(&sprites32_bmp_raw_data32[i]);
    CC_SetVRAMData(&value, offset, sizeof(value));
    offset += sizeof(value);
  }
  sprites32_offset = SPRITES16_BMP_RAW_DATA_SIZE;
}

// These are the number of sprites in the sprite image files.
#define NUM_SPRITE16_TYPES     9
#define NUM_SPRITE32_TYPES     4
#define NUM_SPRITE_TYPES     (NUM_SPRITE16_TYPES + NUM_SPRITE32_TYPES)

// Sprite sizes in total number of pixels/bytes.
#define SPRITE16_SIZE         (16 * 16)
#define SPRITE32_SIZE         (32 * 32)

#define MAX_SPRITE_SIZE             32

#define WORLD_SIZE                 512      // Tile size * map size
#define SCREEN_WIDTH               320      // Screen size.
#define SCREEN_HEIGHT              240

// Sprite speed range
#define MIN_SPEED                    4
#define MAX_SPEED                   64

#define FRAME_RATE                  60      // Animation FPS.

#define abs(x) (((x) > 0) ? (x) : -(x))
#define sign(x) (((x) >= 0) ? 1 : -1)

typedef struct sprite_t {
  int16_t x, y;                // Location.
  int16_t dx, dy;              // Sprite speed.
  int16_t step_x, step_y;      // |dx| and |dy| steps are taken per frame.
                               // When the step counter reaches |FRAME_RATE|,
                               // the location is updated by one.
} Sprite;

extern uint8_t __bss_end;
extern uint8_t __stack;

int main (void) {
  system_init();
  CC_Init();
  printf("Stack ranges from %u to %u\n", &__bss_end, &__stack);

  setup_palette();
  setup_vram();

  // Draw some sprites in various orientations.
  #define NUM_SPRITES_DRAWN 128
  Sprite sprites[NUM_SPRITES_DRAWN];
printf("sprites at %u, %u bytes\n", sprites, sizeof(sprites));
  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    // Randomly generate some sprites.
    Sprite* current = &sprites[i];
    current->x = rand() % WORLD_SIZE;
    current->y = rand() % WORLD_SIZE;
    current->dx = rand() % (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
    current->dy = rand() % (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
    if (rand() % 2)
        current->dx *= -1;
    if (rand() % 2)
        current->dy *= -1;
    current->step_x = 0;
    current->step_y = 0;
  }

  // Wait for vertical refresh before updating sprites.
  while(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK));
  while(!(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK)));

  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    uint16_t sprite_ctrl0_value = (1 << SPRITE_ENABLED) |
                                  (1 << SPRITE_ENABLE_TRANSP) |
                                  (1 << SPRITE_ENABLE_SCROLL);

    // Randomly choose an orientation.
    if (i & 1)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_X);
    if (i & 2)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_Y);
    if (i & 4)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_XY);

    CC_Sprite_SetRegister(i, SPRITE_CTRL0, sprite_ctrl0_value);

    // Determine what kind of sprite.
    int sprite_type = rand() % NUM_SPRITE_TYPES;
    if (sprite_type < NUM_SPRITE16_TYPES) {
      // Set to 16x16.
      CC_Sprite_SetRegister(i, SPRITE_CTRL1,
                           (1 << SPRITE_HSIZE_0) | (1 << SPRITE_VSIZE_0));
      CC_Sprite_SetRegister(i, SPRITE_DATA_OFFSET,
                            sprites16_offset + sprite_type * SPRITE16_SIZE);
    } else {
      sprite_type -= NUM_SPRITE16_TYPES;
      // Set to 32x32.
      CC_Sprite_SetRegister(i, SPRITE_CTRL1,
                           (1 << SPRITE_HSIZE_1) | (1 << SPRITE_VSIZE_1));
      CC_Sprite_SetRegister(i, SPRITE_DATA_OFFSET,
                            sprites32_offset + sprite_type * SPRITE32_SIZE);
    }

    CC_Sprite_SetRegister(i, SPRITE_COLOR_KEY, 0xff);
    CC_Sprite_SetRegister(i, SPRITE_OFFSET_X, sprites[i].x);
    CC_Sprite_SetRegister(i, SPRITE_OFFSET_Y, sprites[i].y);
  }
  CC_SetRegister(CC_REG_SPRITE_Z, 3);

  while (1) {
    while(!(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK)));

    // Continuously move the sprites.
    for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
      Sprite* current = &sprites[i];
      current->step_x += current->dx;
      current->step_y += current->dy;
      while (abs(current->step_x) >= FRAME_RATE) {
        current->step_x -= sign(current->step_x) * FRAME_RATE;
        current->x += sign(current->dx);
      }
      while (abs(current->step_y) >= FRAME_RATE) {
        current->step_y -= sign(current->step_y) * FRAME_RATE;
        current->y += sign(current->dy);
      }

      // Adjust the sprites if they move off screen -- shift them to the other
      // side so they re-enter the visible area quickly.  This way they spend
      // less time in the off-screen area, so the on-sprite density is higher.
      if (current->x > SCREEN_WIDTH && current->dx > 0)
        current->x -= (SCREEN_WIDTH + MAX_SPRITE_SIZE);
      if (current->x < -MAX_SPRITE_SIZE && current->dx < 0)
        current->x += (SCREEN_WIDTH + MAX_SPRITE_SIZE);

      if (current->y > SCREEN_HEIGHT && current->dy > 0)
        current->y -= (SCREEN_HEIGHT + MAX_SPRITE_SIZE);
      else if (current->y < -MAX_SPRITE_SIZE && current->dy < 0)
        current->y += (SCREEN_HEIGHT + MAX_SPRITE_SIZE);

      CC_Sprite_SetRegister(i, SPRITE_OFFSET_X, current->x);
      CC_Sprite_SetRegister(i, SPRITE_OFFSET_Y, current->y);
    }

    while(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK));
  }

  return 0;
}