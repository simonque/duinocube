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

#define BG_TILEMAP_INDEX            0
#define DOTS_TILEMAP_INDEX          1

#define BG_PALETTE_INDEX            0
#define SPRITE_PALETTE_INDEX        1

// TODO: Add this to the DuinoCube library.
#define DEFAULT_EMPTY_TILE_VALUE     0x1fff

// VRAM offsets of background and sprite image data.
uint16_t g_bg_offset;
uint16_t g_sprite_offset;

// Resources to load:
// 1. BG tilemap.
// 2. BG tileset.
// 3. BG palette.
// 4. Sprites.
// 5. Sprite palette.

struct File {
  const char* filename;
  uint16_t* vram_offset;  // For image data, compute and store VRAM offset here.
  uint16_t addr;          // For other data, store data here.
  uint16_t bank;          // Bank to use for |addr|.
  uint16_t max_size;      // Size checking to avoid overflow.
};

const char kFilePath[] = "pacman";    // Base path of data files.

const File kFiles[] = {
  { "bg.lay", NULL, TILEMAP(BG_TILEMAP_INDEX), TILEMAP_BANK, TILEMAP_SIZE },
  { "dots.lay", NULL, TILEMAP(DOTS_TILEMAP_INDEX), TILEMAP_BANK, TILEMAP_SIZE },
  { "tileset.raw", &g_bg_offset, 0, 0, VRAM_BANK_SIZE },
  { "tileset.pal", NULL, PALETTE(BG_PALETTE_INDEX), 0, PALETTE_SIZE },
  { "sprites.raw", &g_sprite_offset, 0, 0, VRAM_BANK_SIZE },
  { "sprites.pal", NULL, PALETTE(SPRITE_PALETTE_INDEX), 0, PALETTE_SIZE },
};

// Sprite definitions.
enum SpriteState {
  SPRITE_DEAD,
  SPRITE_ALIVE,
};

enum SpriteDirection {
  SPRITE_UP,
  SPRITE_DOWN,
  SPRITE_LEFT,
  SPRITE_RIGHT,
};

#define NUM_GHOSTS            4

// Sprite dimensions in pixels.
#define SPRITE_WIDTH         16
#define SPRITE_HEIGHT        16
#define SPRITE_SIZE    (SPRITE_WIDTH * SPRITE_HEIGHT)

// Tile dimensions in pixels.
#define TILE_WIDTH            8
#define TILE_HEIGHT           8
#define TILE_SIZE      (TILE_WIDTH * TILE_HEIGHT)

// Sprite start locations.
#define PLAYER_START_X       (15 * TILE_WIDTH)
#define PLAYER_START_Y       (23 * TILE_HEIGHT)

// Ghost start locations.
#define GHOST_START_X0       (11 * TILE_WIDTH)
#define GHOST_START_DX       ( 2 * TILE_WIDTH)
#define GHOST_START_Y        (11 * TILE_HEIGHT)

// Sprite data offset locations.
#define PLAYER_SPRITE_BASE_OFFSET          0
#define GHOST_SPRITE_BASE_OFFSET           (6 * SPRITE_SIZE)
#define NUM_FRAMES_PER_GHOST               6

// Transparent pixel value.
#define SPRITE_COLOR_KEY                   0

// Sprite size values to write to DuinoCube Core.
// TODO: Make these part of the DuinoCube library.
#define SPRITE_WIDTH_16       (1 << SPRITE_HSIZE_0)
#define SPRITE_HEIGHT_16      (1 << SPRITE_VSIZE_0)

// Sprite Z-depth. This sets it above all tile layers.
#define SPRITE_Z_DEPTH                     3

// Sprite data structure definition.
struct Sprite {
  uint8_t state;                // Alive or dead.
  uint8_t dir;                  // Direction sprite is facing.
  uint16_t x, y;                // Location in pixels.

  uint16_t base_offset;         // Base VRAM offset of sprite's frame images.
  uint8_t frame;                // Animation counter.
  uint8_t size;                 // Size of each sprite frame image in bytes.

  // Compute the sprite's current VRAM offset.
  inline uint16_t get_offset() const {
    return base_offset + frame * size;
  }
};

// Actual sprite objects.
Sprite g_player;
Sprite g_ghosts[NUM_GHOSTS];
// Array of all sprites.
Sprite* sprites[NUM_GHOSTS + 1];

// Load image, palette, and tilemap data from file system.
static void loadResources() {
  uint16_t vram_offset = 0;
  for (int i = 0; i < sizeof(kFiles) / sizeof(kFiles[0]); ++i) {
    const File& file = kFiles[i];

    char filename[256];
    sprintf(filename, "%s/%s", kFilePath, file.filename);

    // Open the file.
    uint16_t handle = DC.File.open(filename, FILE_READ_ONLY);
    if (!handle) {
      printf("Could not open file %s.\n", filename);
      continue;
    }

    uint16_t file_size = DC.File.size(handle);
    printf("File %s is 0x%x bytes\n", filename, file_size);

    if (file_size > file.max_size) {
      printf("File is too big!\n");
      DC.File.close(handle);
      continue;
    }

    // Compute write destination.
    uint16_t dest_addr;
    uint16_t dest_bank;

    if (file.vram_offset) {
      // Set up for VRAM write.

      // If this doesn't fit in the remaining part of the current bank, use the
      // next VRAM bank.
      if (vram_offset % VRAM_BANK_SIZE + file_size > VRAM_BANK_SIZE)
        vram_offset += VRAM_BANK_SIZE - (vram_offset % VRAM_BANK_SIZE);

      // Record VRAM offset.
      *file.vram_offset = vram_offset;

      // Determine the destination VRAM address and bank.
      dest_addr = VRAM_BASE + vram_offset % VRAM_BANK_SIZE;
      dest_bank = vram_offset / VRAM_BANK_SIZE + VRAM_BANK_BEGIN;
      DC.Core.writeWord(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));

      // Update the VRAM offset.
      vram_offset += file_size;
    } else {
      // Set up for non-VRAM write.
      dest_addr = file.addr;
      dest_bank = file.bank;
    }

    printf("Writing to 0x%x with bank = %d\n", dest_addr, dest_bank);
    DC.Core.writeWord(REG_MEM_BANK, dest_bank);
    DC.File.readToCore(handle, dest_addr, file_size);

    DC.File.close(handle);
  }

  // Allow the graphics pipeline access to VRAM.
  DC.Core.writeWord(REG_SYS_CTRL, (0 << REG_SYS_CTRL_VRAM_ACCESS));
}

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
    ghost.dir = SPRITE_RIGHT;
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

  // TODO: Enable game logic.
}

void loop() {
}
