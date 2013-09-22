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

// Sprite drawing demo.

#include <DuinoCube.h>
#include <SPI.h>

// Files to load.
const char* image_files[] = {
  "data/sprite16.raw",
  "data/sprite32.raw",
};

const char* palette_files[] = {
  "data/sprites.pal",
};

static FILE uart_stdout;  // For linking UART to printf, etc.
static int uart_putchar (char c, FILE *stream) {
  Serial.write(c);
  return 0;
}

static uint16_t sprites16_offset = 0;
static uint16_t sprites32_offset = 0;
static uint16_t* sprites_offsets[] = { &sprites16_offset, &sprites32_offset };

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
#define MAX_SPEED                   32

#define FRAME_RATE                  60      // Animation FPS.

#define abs(x) (((x) > 0) ? (x) : -(x))
#define sign(x) (((x) >= 0) ? 1 : -1)

// TODO: Increase the number of sprites.  The Arduino Uno has only 2KB of RAM.
// Need to find a creative way to increase the sprite count without exceeding.
// that limit.
#define NUM_SPRITES_DRAWN           64

struct Sprite {
  int16_t x, y;                // Location.
  int16_t dx, dy;              // Sprite speed.
  int16_t step_x, step_y;      // |dx| and |dy| steps are taken per frame.
                               // When the step counter reaches |FRAME_RATE|,
                               // the location is updated by one.
};
static Sprite sprites[NUM_SPRITES_DRAWN];

// Copy graphics data from file system to Core.
static void load_data() {
  DC.Core.writeWord(REG_MEM_BANK, 0);
  DC.Core.writeWord(REG_SYS_CTRL, 0);

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
  }

  // Load images.
  printf("Loading images.\n");
  uint16_t addr = VRAM_BASE;
  for (int i = 0; i < sizeof(image_files) / sizeof(image_files[0]); ++i) {
    const char* filename = image_files[i];
    uint16_t handle = DC.File.open(filename, 0x01);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }
    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);
    DC.Core.writeWord(REG_MEM_BANK, VRAM_BANK_BEGIN);
    DC.Core.writeWord(REG_SYS_CTRL, 1);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, addr, file_size), addr);
    DC.Core.writeWord(REG_MEM_BANK, 0);
    DC.Core.writeWord(REG_SYS_CTRL, 0);

    // Store the offset of the image that was loaded, and update the address to
    // point to where the next image will be loaded.
    // TODO: Make sure this does not overrun the VRAM bank.
    // .e. if |addr| >= |VRAM_BASE + BANK_SIZE|.
    *sprites_offsets[i] = addr - VRAM_BASE;
    addr += file_size;

    DC.File.close(handle);
  }
}

// Generate various sprites with random locations, directions, and speeds,
static void setup_sprites() {
  // Turn off tile layers.
  for (int i = 0; i < NUM_TILE_LAYERS; ++i)
    DC.Core.writeWord(TILE_LAYER_REG(i, TILE_CTRL_0), 0);

  // Turn off sprites in case there are unused sprites that are still active.
  for (int i = 0; i < NUM_SPRITES; ++i)
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0), 0);

  // Start camera at (0, 0).
  DC.Core.writeWord(REG_SCROLL_X, 0);
  DC.Core.writeWord(REG_SCROLL_Y, 0);

  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    // Randomly generate some sprites.
    Sprite& sprite = sprites[i];
    sprite.x = rand() % WORLD_SIZE;
    sprite.y = rand() % WORLD_SIZE;
    sprite.dx = rand() % (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
    sprite.dy = rand() % (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
    if (rand() % 2)
        sprite.dx *= -1;
    if (rand() % 2)
        sprite.dy *= -1;
    sprite.step_x = 0;
    sprite.step_y = 0;
  }

  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    uint16_t sprite_ctrl0_value = (1 << SPRITE_ENABLED) |
                                  (1 << SPRITE_ENABLE_TRANSP) |
                                  (1 << SPRITE_ENABLE_SCROLL);

    // Select an orientation.
    if (i & 1)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_X);
    if (i & 2)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_Y);
    if (i & 4)
      sprite_ctrl0_value |= (1 << SPRITE_FLIP_XY);

    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0), sprite_ctrl0_value);

    // Determine what size sprite it is (16x16 vs 32x32).
    int sprite_type = rand() % NUM_SPRITE_TYPES;
    if (sprite_type < NUM_SPRITE16_TYPES) {
      // Set to 16x16.
      DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_1),
                        (1 << SPRITE_HSIZE_0) | (1 << SPRITE_VSIZE_0));
      uint16_t offset = sprites16_offset + sprite_type * SPRITE16_SIZE;
      DC.Core.writeWord(SPRITE_REG(i, SPRITE_DATA_OFFSET), offset);
      printf("Sprite with data offset 0x%04x\n", offset);
    } else {
      sprite_type -= NUM_SPRITE16_TYPES;
      // Set to 32x32.
      DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_1),
                        (1 << SPRITE_HSIZE_1) | (1 << SPRITE_VSIZE_1));
      uint16_t offset = sprites32_offset + sprite_type * SPRITE32_SIZE;
      DC.Core.writeWord(SPRITE_REG(i, SPRITE_DATA_OFFSET), offset);
      printf("Sprite with data offset 0x%04x\n", offset);
    }

    DC.Core.writeWord(SPRITE_REG(i, SPRITE_COLOR_KEY), 0xff);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprites[i].x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprites[i].y);
  }
}

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

void setup() {
  Serial.begin(115200);
  fdev_setup_stream(&uart_stdout, uart_putchar, NULL,
  _FDEV_SETUP_WRITE);
  stdout = &uart_stdout;

  // This shows the amount of unallocated memory.  It is helpful in optimizing
  // data storage to fit in the available space.
  printf("Stack ranges from 0x%x to 0x%x\n", &__bss_end, &__stack);

  DC.begin();

  load_data();
  setup_sprites();
}

void loop() {
  // Wait for the next non-Vblank period.
  while(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK));

  // Continuously move the sprites.
  // Update the sprite location values for the next frame.  Compute this
  // during the non-blanking period.  Note that the actual sprites are NOT
  // being updated here, as they are being drawn to the screen.
  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    Sprite& sprite = sprites[i];
    sprite.step_x += sprite.dx;
    sprite.step_y += sprite.dy;
    while (abs(sprite.step_x) >= FRAME_RATE) {
      sprite.step_x -= sign(sprite.step_x) * FRAME_RATE;
      sprite.x += sign(sprite.dx);
    }
    while (abs(sprite.step_y) >= FRAME_RATE) {
      sprite.step_y -= sign(sprite.step_y) * FRAME_RATE;
      sprite.y += sign(sprite.dy);
    }

    // Adjust the sprites if they move off screen -- shift them to the other
    // side so they re-enter the visible area quickly.  This way they spend
    // less time in the off-screen area, so the on-sprite density is higher.
    if (sprite.x > SCREEN_WIDTH && sprite.dx > 0)
      sprite.x -= (SCREEN_WIDTH + MAX_SPRITE_SIZE);
    if (sprite.x < -MAX_SPRITE_SIZE && sprite.dx < 0)
      sprite.x += (SCREEN_WIDTH + MAX_SPRITE_SIZE);

    if (sprite.y > SCREEN_HEIGHT && sprite.dy > 0)
      sprite.y -= (SCREEN_HEIGHT + MAX_SPRITE_SIZE);
    else if (sprite.y < -MAX_SPRITE_SIZE && sprite.dy < 0)
      sprite.y += (SCREEN_HEIGHT + MAX_SPRITE_SIZE);
  }

  // Wait for the next Vblank.
  while(!DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK));

  // Write the new sprite location values to sprite registers.
  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    Sprite& sprite = sprites[i];
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprite.x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprite.y);
  }
}