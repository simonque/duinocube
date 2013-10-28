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

// Universal #define for debugging logging.
#define DEBUG

// Enable this to print stats about loading data from file system.
#define LOG_LOADING

// For measuring the time spent in each cycle.  Important to know if loop()
// takes 1 or 2 frames.
#define LOG_TIMING

// Compare the debug comments with and without this define to see how much of a
// speedup comes from using fast sprite location updates.
#define FAST_SPRITE_LOCATIONS

// Test hardware collision detection.
#define TEST_COLLISION

// Files to load.
const char* image_files[] = {
  "data/sprite16.raw",
  "data/sprite32.raw",
};

const char* palette_files[] = {
  "data/sprites.pal",
};

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

#define NUM_SPRITES_DRAWN          256

// Contains the location of a sprite.
struct SpriteLocation {
  int16_t x, y;                // Location.
};
static SpriteLocation sprite_locations[NUM_SPRITES_DRAWN];

// Contains data used to keep track of a sprite's movement.
// This is separate from the location struct because there is not enough memory
// on the Arduino Uno to store both for the max number of sprites.  So the
// movement data will have to be stored in the DuinoCube's shared memory.
// The fields are chosen as int8_t's to be as small as possible, to reduce time
// spent copying to/from shared memory.
struct SpriteMovement {
  int8_t dx, dy;               // Sprite speed.
  int8_t step_x, step_y;       // |dx| and |dy| steps are taken per frame.
                               // When the step counter reaches |FRAME_RATE|,
                               // the location is updated by one.
};
// Address in shared memory for storing sprite movement data.
static uint16_t sprite_movement_addr;

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
#if defined(DEBUG) && defined(LOG_LOADING)
    printf("File %s is 0x%x bytes\n", filename, file_size);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, PALETTE(i), file_size), PALETTE(i));
#endif
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
    DC.Core.writeWord(REG_MEM_BANK, VRAM_BANK_BEGIN);
    DC.Core.writeWord(REG_SYS_CTRL, 1);
#if defined(DEBUG) && defined(LOG_LOADING)
    printf("File %s is 0x%x bytes\n", filename, file_size);
    printf("Wrote 0x%x bytes to 0x%x\n",
           DC.File.readToCore(handle, addr, file_size), addr);
#endif
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

  // Allocate movement data.
  sprite_movement_addr =
      DC.Mem.alloc(sizeof(SpriteMovement) * NUM_SPRITES_DRAWN);
  if (!sprite_movement_addr) {
    printf("Unable to allocate %u bytes for sprite movement data.\n",
           sizeof(SpriteMovement) * NUM_SPRITES_DRAWN);
    return;
  }

  uint16_t addr = sprite_movement_addr;
  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i, addr += sizeof(SpriteMovement)) {
    // Randomly generate some sprites.
    SpriteLocation& sprite = sprite_locations[i];
    sprite.x = rand() % WORLD_SIZE;
    sprite.y = rand() % WORLD_SIZE;

    // Select speed and direction for each sprite.
    SpriteMovement movement;
    movement.dx = rand() % (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
    movement.dy = rand() % (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
    if (rand() % 2)
      movement.dx *= -1;
    if (rand() % 2)
      movement.dy *= -1;
    movement.step_x = 0;
    movement.step_y = 0;

    // Store movement data in shared memory.
    DC.Sys.writeSharedRAM(addr, &movement, sizeof(movement));
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
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), sprite_locations[i].x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), sprite_locations[i].y);
  }
}

// Read multiple sprite movement structs at a time, to reduce the time spent
// sending control bytes vs data bytes.
#define NUM_MOVEMENTS_TO_READ    64
SpriteMovement movements[NUM_MOVEMENTS_TO_READ];

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

void setup() {
  Serial.begin(115200);

  DC.begin();

  // This shows the amount of unallocated memory.  It is helpful in optimizing
  // data storage to fit in the available space.
  printf("Stack ranges from 0x%x to 0x%x\n", &__bss_end, &__stack);

  load_data();
  setup_sprites();
}

void loop() {
#if defined(DEBUG) && defined(LOG_TIMING)
  static uint32_t t0 = 0;
#endif

  // Wait for the next non-Vblank period.
  while (DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK));

#if defined(DEBUG) && defined(LOG_TIMING)
  uint32_t t1 = millis();
#endif

  // Continuously move the sprites.
  // Update the sprite location values for the next frame.  Compute this
  // during the non-blanking period.  Note that the actual sprites are NOT
  // being updated here, as they are being drawn to the screen.
  uint16_t addr = sprite_movement_addr;
  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i, addr += sizeof(SpriteMovement)) {
    // Load movement data from shared memory.
    if (i % NUM_MOVEMENTS_TO_READ == 0)
      DC.Sys.readSharedRAM(addr, movements, sizeof(movements));
    SpriteMovement& movement = movements[i % NUM_MOVEMENTS_TO_READ];

    // Update the location and movement counter.
    SpriteLocation& location = sprite_locations[i];
    movement.step_x += movement.dx;
    movement.step_y += movement.dy;
    while (abs(movement.step_x) >= FRAME_RATE) {
      movement.step_x -= sign(movement.step_x) * FRAME_RATE;
      location.x += sign(movement.dx);
    }
    while (abs(movement.step_y) >= FRAME_RATE) {
      movement.step_y -= sign(movement.step_y) * FRAME_RATE;
      location.y += sign(movement.dy);
    }

    // Write the updated movement data back to shared memory.
    DC.Sys.writeSharedRAM(addr, &movement, sizeof(movement));

    // Adjust the sprites if they move off screen -- shift them to the other
    // side so they re-enter the visible area quickly.  This way they spend
    // less time in the off-screen area, so the on-sprite density is higher.
    if (location.x > SCREEN_WIDTH && movement.dx > 0)
      location.x -= (SCREEN_WIDTH + MAX_SPRITE_SIZE);
    if (location.x < -MAX_SPRITE_SIZE && movement.dx < 0)
      location.x += (SCREEN_WIDTH + MAX_SPRITE_SIZE);

    if (location.y > SCREEN_HEIGHT && movement.dy > 0)
      location.y -= (SCREEN_HEIGHT + MAX_SPRITE_SIZE);
    else if (location.y < -MAX_SPRITE_SIZE && movement.dy < 0)
      location.y += (SCREEN_HEIGHT + MAX_SPRITE_SIZE);
  }

#ifdef TEST_COLLISION
  uint16_t collision_addr = COLLISION_BASE;
  struct {
    uint8_t target_index;
    uint8_t collided;
  } collision;
  for (uint16_t i = 0;
       i < NUM_SPRITES_DRAWN;
       ++i, collision_addr += sizeof(collision)) {
    DC.Core.readData(collision_addr, &collision, sizeof(collision));
    if (!collision.collided || i == collision.target_index)
      continue;
#ifdef DEBUG
    printf("Collision between sprites %3d and %3d\n",
           i, collision.target_index);
#endif
    // If there was a collision, remove both of the sprites.
    // For speed, handle no more than one collision per refresh cycle.
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_CTRL_0), 0);
    DC.Core.writeWord(SPRITE_REG(collision.target_index, SPRITE_CTRL_0), 0);
    break;
  }
#endif  // defined(TEST_COLLISION)

#if defined(DEBUG) && defined(LOG_TIMING)
  uint32_t t2 = millis();
  printf("Computation time: %lu ms\n", t2 - t1);
#endif

  // Wait for the next Vblank.
  while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

#if defined(DEBUG) && defined(LOG_TIMING)
  uint32_t t3 = millis();
#endif

  // Write the new sprite location values to sprite registers.
#ifdef FAST_SPRITE_LOCATIONS
  DC.Core.writeData(SPRITE_XY_BASE, sprite_locations, sizeof(sprite_locations));
#else
  for (int i = 0; i < NUM_SPRITES_DRAWN; ++i) {
    SpriteLocation& location = sprite_locations[i];
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_X), location.x);
    DC.Core.writeWord(SPRITE_REG(i, SPRITE_OFFSET_Y), location.y);
  }
#endif  // defined(FAST_SPRITE_LOCATIONS)

#if defined(DEBUG) && defined(LOG_TIMING)
  uint32_t t4 = millis();
  printf("Update time: %lu ms\n", t4 - t3);
  printf("Cycle time: %lu ms\n", t4 - t0);
  t0 = t4;
#endif
}
