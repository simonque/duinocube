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

// Test DuinoCube tile layers, sprites, and USB input.

#include <DuinoCube.h>
#include <SPI.h>

#if defined(__AVR_ATmega32U4__)
#include <Esplora.h>
#endif

// Files to load.
const char* image_files[] = {
  "data/tileset.raw",
  "data/clouds.raw",
  "data/sprite32.raw",
};

const char* palette_files[] = {
  "data/tileset.pal",
  "data/clouds.pal",
  "data/sprites.pal",
};

const char* layer_files[] = {
  "data/desert0.lay",
  "data/desert1.lay",
  "data/desert2.lay",
  "data/clouds.lay",
};

// The order of these should match the order of image data being loaded.
static uint16_t landscape_offset, clouds_offset, sprites_offset;
static uint16_t* vram_offsets[] = {
  &landscape_offset,
  &clouds_offset,
  &sprites_offset,
};

// The order of these should match the order of palette data being loaded.
static uint8_t landscape_pal, clouds_pal, sprites_pal;
static uint8_t* palettes[] = { &landscape_pal, &clouds_pal, &sprites_pal };

static void load() {
  DC.Core.writeWord(REG_MEM_BANK, 0);
  DC.Core.writeWord(REG_SYS_CTRL, 0);

  // Reset tile layers and sprites.
  for (int i = 0; i < NUM_TILE_LAYERS; ++i)
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_CTRL_0), 0);
  for (int i = 0; i < NUM_SPRITES; ++i)
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0), 0);

  // Load palettes.
  printf("Loading palettes.\n");
  for (int i = 0; i < sizeof(palette_files) / sizeof(palette_files[0]); ++i) {
    const char* filename = palette_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, PALETTE(i), file_size), PALETTE(i));
    DC.File.close(handle);

    *palettes[i] = i;
  }

  // Load layers.
  printf("Loading layers.\n");
  printf("Layers: %d\n", sizeof(layer_files) / sizeof(layer_files[0]));
  for (int i = 0; i < sizeof(layer_files) / sizeof(layer_files[0]); ++i) {
    const char* filename = layer_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    DC.Core.writeWord(REG_MEM_BANK, TILEMAP_BANK);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, TILEMAP(i), file_size), TILEMAP(i));
    DC.Core.writeWord(REG_MEM_BANK, 0);
    DC.File.close(handle);
  }

  // Load images.
  printf("Loading images.\n");
  uint16_t addr = VRAM_BASE;
  uint16_t bank = VRAM_BANK_BEGIN;
  for (int i = 0; i < sizeof(image_files) / sizeof(image_files[0]); ++i) {
    const char* filename = image_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    DC.Core.writeWord(REG_MEM_BANK, bank);
    DC.Core.writeWord(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));
    printf("Wrote 0x%x bytes to 0x%x, bank = %u\n",
           DC.File.readToCore(handle, addr, file_size), addr, bank);
    DC.Core.writeWord(REG_MEM_BANK, 0);
    DC.Core.writeWord(REG_SYS_CTRL, 0);

    *vram_offsets[i] =
        (addr - VRAM_BASE) + VRAM_BANK_SIZE * (bank - VRAM_BANK_BEGIN);

    addr += file_size;
    // TODO: this assumes that the individual data reads won't cross the bank
    // boundaries.
    while (addr >= VRAM_BASE + VRAM_BANK_SIZE) {
      addr -= VRAM_BANK_SIZE;
      bank += 1;
    }
    DC.File.close(handle);
  }
}

// Contains the location of a sprite.
struct Sprite {
  int16_t x, y;                // Location.
  int8_t dx, dy;               // Sprite speed.
  int8_t step_x, step_y;       // |dx| and |dy| steps are taken per frame.
  uint16_t ctrl0_value;        // Value of the SPRITE_CTRL_0 register.
  uint16_t offset;             // Offset of image data.
};

static Sprite player_sprite;

static void draw() {
  // Set up tile layer registers.
  // TODO: explain value.
  word landscape_tile_ctrl0_value =
      (1 << TILE_LAYER_ENABLED) |
      (1 << TILE_ENABLE_NOP) |
      (1 << TILE_ENABLE_TRANSP) |
      (1 << TILE_ENABLE_FLIP) |
      (landscape_pal << TILE_PALETTE_START);
  word clouds_tile_ctrl0_value =
      (1 << TILE_LAYER_ENABLED) |
      (1 << TILE_ENABLE_NOP) |
      (1 << TILE_ENABLE_TRANSP) |
      (1 << TILE_ENABLE_FLIP) |
      (clouds_pal << TILE_PALETTE_START);

  struct TileLayerInfo {
    uint16_t ctrl0, empty, color_key, offset;
  };

  const TileLayerInfo layer_info[] = {
    // TODO: explain the meaning of the values.
    { landscape_tile_ctrl0_value, 0x1fff, 0xff, landscape_offset },
    { landscape_tile_ctrl0_value, 0x1fff, 0xff, landscape_offset },
    { landscape_tile_ctrl0_value, 0x1fff, 0xff, landscape_offset },
    { clouds_tile_ctrl0_value, 0x1fff, 0xff, clouds_offset },
  };

  for (int i = 0; i < sizeof(layer_files) / sizeof(layer_files[0]); ++i) {
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_CTRL_0), layer_info[i].ctrl0);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_EMPTY_VALUE), layer_info[i].empty);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_COLOR_KEY),
                      layer_info[i].color_key);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_DATA_OFFSET),
                      layer_info[i].offset);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_OFFSET_X), 0);
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_OFFSET_Y), 0);
  }

  // Set up sprite.
  uint16_t sprite_index = 0;

  // Set to 32x32 size.
  DC.Core.writeWord(SPRITE_REG(sprite_index, SPRITE_CTRL_1),
                    (1 << SPRITE_HSIZE_1) | (1 << SPRITE_VSIZE_1));

  // Set image data offset.
  DC.Core.writeWord(SPRITE_REG(sprite_index, SPRITE_DATA_OFFSET),
                    sprites_offset);

  // Set transparency.
  DC.Core.writeWord(SPRITE_REG(sprite_index, SPRITE_COLOR_KEY), 0xff);

  // Set location.
  player_sprite.x = 0;
  player_sprite.y = 0;
  DC.Core.writeWord(SPRITE_REG(sprite_index, SPRITE_OFFSET_X), player_sprite.x);
  DC.Core.writeWord(SPRITE_REG(sprite_index, SPRITE_OFFSET_Y), player_sprite.y);

  // Finally, turn on sprite.
  uint16_t sprite_ctrl0_value = (1 << SPRITE_ENABLED) |
                                (1 << SPRITE_ENABLE_TRANSP) |
                                (1 << SPRITE_ENABLE_SCROLL) |
                                (sprites_pal << SPRITE_PALETTE_START);
  DC.Core.writeWord(SPRITE_REG(sprite_index, SPRITE_CTRL_0),
                    sprite_ctrl0_value);
}

void setup() {
  Serial.begin(115200);

  DC.begin();

  load();
  draw();
}

#define SCREEN_WIDTH          320       // Screen dimensions.
#define SCREEN_HEIGHT         240       // TODO: de-hardcode this.

#define SPRITE_WIDTH           32       // Dimensions of 32x32 sprite.
#define SPRITE_HEIGHT          32
#define SPRITE_SIZE        (SPRITE_WIDTH * SPRITE_HEIGHT)
#define NUM_SPRITE_IMAGES       4

#define UINT8_MAX     ((uint8_t)(~0))

#define MAX_MOVEMENT_SPEED      3

void loop() {
  // Start camera at (0, 0).
  DC.Core.writeWord(REG_SCROLL_X, 0);
  DC.Core.writeWord(REG_SCROLL_Y, 0);

  int16_t scroll_x = 0;
  int16_t scroll_y = 0;

  GamepadState prev_gamepad;
  prev_gamepad.buttons = 0;
  prev_gamepad.x = UINT8_MAX / 2;
  prev_gamepad.y = UINT8_MAX / 2;

  uint16_t old_flip_flags = 0;

  int8_t dx = 0;
  int8_t dy = 0;

  uint8_t sprite_z = 3;

  const int step = 8;
  for (uint16_t i = 0; ; i += step) {
    // Wait for visible, non-vblanked region to do computations.
    while ((DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

    // Read user input.
    GamepadState gamepad = DC.Gamepad.readGamepad();

    // The four main control buttons select the orientation.
    uint16_t new_flip_flags = old_flip_flags;
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_1)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_1))) {
      new_flip_flags = 0;
    }
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_2)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_2))) {
      new_flip_flags = (1 << SPRITE_FLIP_Y) | (1 << SPRITE_FLIP_XY);
    }
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_3)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_3))) {
      new_flip_flags = (1 << SPRITE_FLIP_Y);
    }
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_4)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_4))) {
      new_flip_flags = (1 << SPRITE_FLIP_XY);
    }
    if (old_flip_flags != new_flip_flags)
      printf("0x%04x -> 0x%04x\n", old_flip_flags, new_flip_flags);

    // Read the SPRITE_CTRL_0 register and clear the flip bits.
    uint16_t sprite_ctrl0_value =
        DC.Core.readWord(SPRITE_REG(0, SPRITE_CTRL_0));
    if (new_flip_flags != old_flip_flags) {
      sprite_ctrl0_value &= ~(1 << SPRITE_FLIP_X);
      sprite_ctrl0_value &= ~(1 << SPRITE_FLIP_Y);
      sprite_ctrl0_value &= ~(1 << SPRITE_FLIP_XY);
      sprite_ctrl0_value |= new_flip_flags;
    }
    old_flip_flags = new_flip_flags;

    // L1 and R1 cycle sprite through different images.
    int sprite_image_index =
        (player_sprite.offset - sprites_offset) / SPRITE_SIZE;
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_L1)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_L1))) {
      --sprite_image_index;
    }
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_R1)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_R1))) {
      ++sprite_image_index;
    }
    // Adjust for valid image index values.
    sprite_image_index =
        (sprite_image_index + NUM_SPRITE_IMAGES) % NUM_SPRITE_IMAGES;
    player_sprite.offset = sprites_offset + sprite_image_index * SPRITE_SIZE;

    // L2 and R2 buttons to change sprite Z-level.
    if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_R2)) &&
        !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_R2)) &&
        sprite_z < NUM_TILE_LAYERS - 1) {
      ++sprite_z;
    } else if ((gamepad.buttons & (1 << GAMEPAD_BUTTON_L2)) &&
               !(prev_gamepad.buttons & (1 << GAMEPAD_BUTTON_L2)) &&
               sprite_z > 0) {
      --sprite_z;
    }

    // Directional pad moves sprite.
    if (gamepad.x == 0) {
      // Use acceleration.
      if (prev_gamepad.x != gamepad.x)
        dx = -1;
      else if (dx > -MAX_MOVEMENT_SPEED)
        --dx;
      player_sprite.x += dx;
    }
    else if (gamepad.x == UINT8_MAX) {
      // Use acceleration.
      if (prev_gamepad.x != gamepad.x)
        dx = 1;
      else if (dx < MAX_MOVEMENT_SPEED)
        ++dx;
      player_sprite.x += dx;
    }

    if (gamepad.y == 0) {
      // Use acceleration.
      if (prev_gamepad.y != gamepad.y)
        dy = -1;
      else if (dy > -MAX_MOVEMENT_SPEED)
        --dy;
      player_sprite.y += dy;
    }
    else if (gamepad.y == UINT8_MAX) {
      // Use acceleration.
      if (prev_gamepad.y != gamepad.y)
        dy = 1;
      else if (dy < MAX_MOVEMENT_SPEED)
        ++dy;
      player_sprite.y += dy;
    }

    // Save the current gamepad state for the next cycle.
    prev_gamepad = gamepad;

    if (player_sprite.x < scroll_x)
      scroll_x = player_sprite.x;
    else if (player_sprite.x >= scroll_x + SCREEN_WIDTH - SPRITE_WIDTH)
      scroll_x = player_sprite.x + SPRITE_WIDTH - SCREEN_WIDTH;

    if (player_sprite.y < scroll_y)
      scroll_y = player_sprite.y;
    else if (player_sprite.y >= scroll_y + SCREEN_HEIGHT - SPRITE_HEIGHT)
      scroll_y = player_sprite.y + SPRITE_HEIGHT - SCREEN_HEIGHT;

    // Update the cloud movement.
    uint16_t clouds_x = (i / 8);
    uint16_t clouds_y = -(i / 16);

    // Wait for Vblank.
    while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

    // Scroll the camera.
    DC.Core.writeWord(REG_SCROLL_X, scroll_x);
    DC.Core.writeWord(REG_SCROLL_Y, scroll_y);

    // Scroll the cloud layer independently.
    DC.Core.writeWord(TILE_LAYER_REG(3, TILE_OFFSET_X), clouds_x);
    DC.Core.writeWord(TILE_LAYER_REG(3, TILE_OFFSET_Y), clouds_y);

    // Update the sprite.
    DC.Core.writeWord(SPRITE_REG(0, SPRITE_CTRL_0), sprite_ctrl0_value);
    DC.Core.writeWord(SPRITE_REG(0, SPRITE_DATA_OFFSET), player_sprite.offset);
    DC.Core.writeWord(SPRITE_REG(0, SPRITE_OFFSET_X), player_sprite.x);
    DC.Core.writeWord(SPRITE_REG(0, SPRITE_OFFSET_Y), player_sprite.y);
    DC.Core.writeWord(16, sprite_z);
  }
}
