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

// Menu + fancy background demo.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc_core.h"
#include "registers.h"
#include "tile_registers.h"

#include "system.h"
#include "spipad.h"

#include "font_8x8.h"
#include "data/spectrum.raw.h"
#include "data/spectrum.pal.h"


#define BG_PALETTE_INDEX     0
#define TEXT_PALETTE_INDEX   1

#define BG_LAYER_INDEX       0
#define TEXT_LAYER_INDEX     1

#define FONT_NONE            0
#define FONT_BLACK           1
#define FONT_WHITE           2

#define NUM_PALETTE_ENTRIES      32       // Number of colors used in palette
#define COLOR_CYCLE_PERIOD        8

static void cycle_colors(int offset) {
  uint32_t value;
  for (uint16_t i = 0; i < NUM_PALETTE_ENTRIES; ++i) {
    // The color palette has two sets of identical 42 colors.  This allows the
    // colors to be cycled without having to manually wrap around.
    value = pgm_read_dword(&spectrum_bmp_pal_data32[i + offset]);
    CC_SetPaletteData(&value, BG_PALETTE_INDEX, i * sizeof(value),
                      sizeof(value));
  }
}

static void setup_palette(void) {
  uint32_t black = 0x00000000;
  uint32_t white = 0xffffffff;
  CC_SetPaletteData(&black, TEXT_PALETTE_INDEX, FONT_BLACK * sizeof(uint32_t),
                    sizeof(black));
  CC_SetPaletteData(&white, TEXT_PALETTE_INDEX, FONT_WHITE * sizeof(uint32_t),
                    sizeof(white));

  cycle_colors(0);
}

static uint16_t font_offset = 0;
static uint16_t bg_offset = 0;

static void setup_vram(void) {
  font_offset = 0;
  uint16_t offset = 0;
  for (int i = 0; i < NUM_FONT_CHARS; ++i) {
    char c = i;
    for (int line = 0; line < NUM_FONT_CHAR_LINES; ++line) {
      uint8_t line_buffer[8];
      memset(line_buffer, 0, sizeof(line_buffer));

      // Draw the shadow in black, offset by (1, 1) pixels.
      if (line > 0) {
        uint8_t line_data = get_font_line(c, line - 1);
        for (int b = 1; b < sizeof(line_buffer); ++b) {
          if (line_data & (1 << (b - 1)))
            line_buffer[b] = FONT_BLACK;
        }
      }

      // Draw the regular character bits in white.
      uint8_t line_data = get_font_line(c, line);
      for (int b = 0; b < sizeof(line_buffer); ++b) {
        if (line_data & (1 << b))
          line_buffer[b] = FONT_WHITE;
      }

      CC_SetVRAMData(line_buffer, offset, sizeof(line_buffer));
      offset += sizeof(line_buffer);
    }
  }

  bg_offset = offset;
  for (int i = 0; i < SPECTRUM_BMP_RAW_DATA_SIZE / sizeof(uint32_t); ++i) {
    uint32_t value = pgm_read_dword(&spectrum_bmp_raw_data32[i]);
    CC_SetVRAMData(&value, offset, sizeof(value));
    offset += sizeof(value);
  }
}

#define TILEMAP_WIDTH     32
#define TILEMAP_HEIGHT    32
static void setup_tilemap(void) {
  // Generate the background tile map.
  // There are two tiles that alternate.  Generate two lines and repeat it.
  uint16_t line_buffer[TILEMAP_WIDTH * 2];
  for (int i = 0; i < TILEMAP_WIDTH; ++i) {
    line_buffer[i] = 0;
    if (i % 2)
      line_buffer[i] |= (1 << 13);
  }
  for (int i = TILEMAP_WIDTH; i < TILEMAP_WIDTH * 2; ++i) {
    line_buffer[i] = (1 << 14);
    if (i % 2)
      line_buffer[i] |= (1 << 13);
  }

  // Copy the two lines into VRAM.
  for (int y = 0; y < TILEMAP_HEIGHT / 2; ++y) {
    CC_TileLayer_SetData(line_buffer, BG_LAYER_INDEX, y * sizeof(line_buffer),
                         sizeof(line_buffer));
  }
}

static void setup_tile_layers(void) {
  CC_TileLayer_SetRegister(BG_LAYER_INDEX, TILE_CTRL0,
                           (1 << TILE_LAYER_ENABLED) |
                           (1 << TILE_ENABLE_FLIP) |
                           (BG_PALETTE_INDEX << TILE_PALETTE_START));
  CC_TileLayer_SetRegister(BG_LAYER_INDEX, TILE_DATA_OFFSET, bg_offset);

  CC_TileLayer_SetRegister(TEXT_LAYER_INDEX, TILE_CTRL0,
                           (1 << TILE_LAYER_ENABLED) |
                           (1 << TILE_ENABLE_8x8) |
                           (1 << TILE_ENABLE_8_BIT) |
                           (1 << TILE_ENABLE_TRANSP) |
                           (TEXT_PALETTE_INDEX << TILE_PALETTE_START));
  CC_TileLayer_SetRegister(TEXT_LAYER_INDEX, TILE_DATA_OFFSET, font_offset);
  CC_TileLayer_SetRegister(TEXT_LAYER_INDEX, TILE_COLOR_KEY, FONT_NONE);
}

extern uint8_t __bss_end;
extern uint8_t __stack;

enum {
  MENU_INVADERS,
  MENU_TETRIS,
  MENU_LANDSCAPE,
  MENU_SPRITES,
  NUM_MENU_ITEMS,
};

#define SCREEN_WIDTH    320

#define TEXT_SIZE         8
#define TITLE_X          (SCREEN_WIDTH / TEXT_SIZE / 2)
#define TITLE_Y           3

#define MENU_X           TITLE_X
#define MENU_Y_START     15
#define MENU_Y_STEP       3

#define TEXT_MAP_WIDTH   64

void render_text(const char* text, int x, int y, int centered) {
  if (centered)
    x -= strlen(text) / 2;
  CC_TileLayer_SetData(text, TEXT_LAYER_INDEX, x + y * TEXT_MAP_WIDTH,
                       strlen(text));
}

static const char* kMenuText[] = {
  "Space Invaders",
  "Tetris",
  "Landscape demo",
  "Sprites demo"
};

static void setup_text(void) {
  render_text("CHRONOCUBE", TITLE_X, TITLE_Y, 1);
  render_text("An open source graphics renderer", TITLE_X, TITLE_Y + 2, 1);
  render_text("by Simon Que", TITLE_X, TITLE_Y + 4, 1);

  render_text("Use UP/DOWN to choose one:", MENU_X, MENU_Y_START - MENU_Y_STEP,
              1);

  for (int i = 0; i < sizeof(kMenuText) / sizeof(kMenuText[0]); ++i)
    render_text(kMenuText[i], MENU_X, MENU_Y_START + i * MENU_Y_STEP, 1);
}

static void erase_menu_selectors(int select) {
  render_text("   ", MENU_X - strlen(kMenuText[select]) / 2 - 4,
              MENU_Y_START + MENU_Y_STEP * select, 1);
  render_text("   ", MENU_X + strlen(kMenuText[select]) / 2 + 3,
              MENU_Y_START + MENU_Y_STEP * select, 1);
}

static void draw_menu_selectors(int select) {
  render_text("==>", MENU_X - strlen(kMenuText[select]) / 2 - 4,
              MENU_Y_START + MENU_Y_STEP * select, 1);
  render_text("<==", MENU_X + strlen(kMenuText[select]) / 2 + 3,
              MENU_Y_START + MENU_Y_STEP * select, 1);
}

int main (void) {
  system_init();
  spipad_init();
  CC_Init();

  printf("Stack ranges from %u to %u\n", &__bss_end, &__stack);

  setup_palette();
  setup_vram();
  setup_tilemap();
  setup_text();
  setup_tile_layers();

  int color_cycle_count = 0;
  int color_cycle_offset = 0;

  int menu_selection = 0;
  SpiPadButtons buttons;
  memset(&buttons, 0, sizeof(buttons));

  while (1) {
    while(!(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK)));

    erase_menu_selectors(menu_selection);

    SpiPadButtons new_buttons = spipad_read();
    if (!buttons.UP && new_buttons.UP) {
      if (menu_selection == 0)
        menu_selection = NUM_MENU_ITEMS;
      --menu_selection;
      buttons.UP = 1;
    } else if (buttons.UP && !new_buttons.UP) {
      buttons.UP = 0;
    }

    if (!buttons.DOWN && new_buttons.DOWN) {
      ++menu_selection;
      if (menu_selection == NUM_MENU_ITEMS)
        menu_selection = 0;
      buttons.DOWN = 1;
    } else if (buttons.DOWN && !new_buttons.DOWN) {
      buttons.DOWN = 0;
    }

    draw_menu_selectors(menu_selection);

    if(++color_cycle_count >= COLOR_CYCLE_PERIOD) {
      color_cycle_count = 0;

      ++color_cycle_offset;
      if (color_cycle_offset >= NUM_PALETTE_ENTRIES)
        color_cycle_offset -= NUM_PALETTE_ENTRIES;

      cycle_colors(color_cycle_offset);
    }

    while(CC_GetRegister(CC_REG_OUTPUT_STATUS) & (1 << CC_REG_VBLANK));
  }

  return 0;
}