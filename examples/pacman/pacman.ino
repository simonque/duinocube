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

// A Pacman port.

#include <DuinoCube.h>
#include <SPI.h>

#if defined(__AVR_Atmega32U4__)
#include <Esplora.h>
#endif

#include "defines.h"
#include "map.h"
#include "resources.h"

#define GHOST_MOVEMENT_SPEED      1

// Actual sprite objects.
Sprite g_player;
Sprite g_ghosts[NUM_GHOSTS];
// Array of all sprites.
Sprite* sprites[NUM_GHOSTS + 1];

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

// Enable background layers.
static void setupLayers() {
  for (int layer_index = 0; layer_index < NUM_TILEMAPS; ++layer_index) {
    // Determine how to handle each layer.
    switch(layer_index) {
    case BG_TILEMAP_INDEX:
    case DOTS_TILEMAP_INDEX:
      // Set up the layer later.
      break;
    default:
      // Disable inactive tile layers.
      DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_CTRL_0), 0);
      continue;
    }

    DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_CTRL_0),
                      (1 << TILE_LAYER_ENABLED) |
                      (1 << TILE_ENABLE_8x8) |
                      (1 << TILE_ENABLE_NOP) |
                      (1 << TILE_ENABLE_FLIP) |
                      (BG_PALETTE_INDEX << TILE_PALETTE_START));
    DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_DATA_OFFSET),
                      g_bg_offset);
    DC.Core.writeWord(TILE_LAYER_REG(layer_index, TILE_EMPTY_VALUE),
                      DEFAULT_EMPTY_TILE_VALUE);
  }
}

// Initialize sprites.
static void setupSprites() {
  // Initialize player sprite.
  g_player.state = SPRITE_ALIVE;
  g_player.dir = SPRITE_RIGHT;
  g_player.x = PLAYER_START_X;
  g_player.y = PLAYER_START_Y;

  g_player.base_offset = g_sprite_offset + PLAYER_SPRITE_BASE_OFFSET;
  g_player.frame = 0;
  g_player.size = SPRITE_SIZE;

  // Initialize ghost sprites.
  for (int i = 0; i < NUM_GHOSTS; ++i) {
    Sprite& ghost = g_ghosts[i];
    ghost.state = SPRITE_ALIVE;
    ghost.dir = (i < NUM_GHOSTS / 2) ? SPRITE_LEFT : SPRITE_RIGHT;
    ghost.x = GHOST_START_X0 + i * GHOST_START_DX;
    ghost.y = GHOST_START_Y;

    ghost.base_offset = g_sprite_offset +
                        GHOST_SPRITE_BASE_OFFSET +
                        i * SPRITE_SIZE * NUM_FRAMES_PER_GHOST;
    ghost.frame = 0;
    ghost.size = SPRITE_SIZE;
  }

  // Set sprite Z-depth.
  DC.Core.writeWord(REG_SPRITE_Z, SPRITE_Z_DEPTH);

  // Set up sprite array.
  sprites[0] = &g_player;
  for (int i = 0; i < NUM_GHOSTS; ++i)
    sprites[i + 1] = &g_ghosts[i];

  // Set up sprite rendering.
  for (int i = 0; i < sizeof(sprites) / sizeof(sprites[0]); ++i) {
    const Sprite& sprite = *sprites[i];

    // Set sprite size.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_1),
                      SPRITE_WIDTH_16 | SPRITE_HEIGHT_16);

    // Set image data offset.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_DATA_OFFSET), sprite.get_offset());

    // Set transparency.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_COLOR_KEY), SPRITE_COLOR_KEY);

    // Set location.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprite.x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprite.y);

    // Enable the sprite.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0),
                      (1 << SPRITE_ENABLED) |
                      (1 << SPRITE_ENABLE_TRANSP) |
                      (1 << SPRITE_ENABLE_SCROLL) |
                      (SPRITE_PALETTE_INDEX << SPRITE_PALETTE_START));
  }
}

void setup() {
  Serial.begin(115200);
  DC.begin();

  while (!Serial.available());

  loadResources();
  setupLayers();
  setupSprites();

  g_directions[SPRITE_UP] = (Vector){ 0, -1 };
  g_directions[SPRITE_DOWN] = (Vector){ 0, 1 };
  g_directions[SPRITE_LEFT] = (Vector){ -1, 0 };
  g_directions[SPRITE_RIGHT] = (Vector){ 1, 0 };

  printf("Static data ends at 0x%04x\n", &__bss_end);
  printf("Stack is at 0x%04x\n", &__stack);

  // Initialize random generator with time.
  // TODO: Use a less deterministic seed value.
  srand(millis());

  // TODO: Enable game logic.
}

void loop() {
  // Wait for visible, non-vblanked region to do computations.
  while ((DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  // Update ghosts.
  for (int i = 0; i < NUM_GHOSTS; ++i) {
    Sprite& ghost = g_ghosts[i];

    // Is ghost at an intersection? If so, randomly choose an available
    // direction that doesn't entail going backwards.
    // TODO: Correctly handle a dead end.
    if (isAtIntersection(ghost)) {
      // Count the number of available directions.
      uint8_t new_dirs[NUM_SPRITE_DIRS];
      uint8_t num_available_dirs = 0;
      for (uint8_t dir = 0; dir < NUM_SPRITE_DIRS; ++dir) {
        if (getOppositeDir(ghost.dir) == dir)  // Do not go backwards.
          continue;

        // Otherwise, if the new direction is not blocked, add it to the list of
        // possible directions.
        const Vector& dir_vector = g_directions[dir];
        if (isEmptyTile(getTileX(ghost.x) + dir_vector.x,
                        getTileY(ghost.y) + dir_vector.y)) {
          new_dirs[num_available_dirs++] = dir;
        }
      }

      // Choose a direction from the list.
      if (num_available_dirs > 0) {
        ghost.dir = new_dirs[rand() % num_available_dirs];
      } else {
        printf("Unable to find a new direction for ghost at (%u, %u)\n",
               ghost.x, ghost.y);
      }
    }

    // Update ghost location.
    Vector& dir_vector = g_directions[ghost.dir];
    ghost.x += dir_vector.x * GHOST_MOVEMENT_SPEED;
    ghost.y += dir_vector.y * GHOST_MOVEMENT_SPEED;
  }

  // Wait for Vblank to update rendering.
  while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  for (int i = 0; i < sizeof(sprites) / sizeof(sprites[0]); ++i) {
    const Sprite& sprite = *sprites[i];
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprite.x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprite.y);
  }
}
