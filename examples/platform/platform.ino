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
#include "resources.h"
#include "sprites.h"

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

namespace {

Sprite sprites[2];
Sprite& bat = sprites[0];
// TODO: The player sprite should be a composite sprite.
Sprite& player = sprites[1];

// Bat animation sequence.
const uint8_t kBatFrames[] = { 0, 1 };

// Initialize sprites.
void initSprites() {
  // Initialize player sprite.
  player.state = SPRITE_ALIVE;
  player.dir = SPRITE_RIGHT;
  player.x = 0;
  player.y = 0;

  player.base_offset = g_player_offset;
  player.size = BAT_SPRITE_SIZE;

  // Initialize a bat sprite.
  bat.state = SPRITE_ALIVE;
  bat.dir = SPRITE_RIGHT;
  bat.x = 0;
  bat.y = 0;

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

  printf("Static data ends at 0x%04x\n", &__bss_end);
  printf("Stack is at 0x%04x\n", &__stack);

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
