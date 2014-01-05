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
