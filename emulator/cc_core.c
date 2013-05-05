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

// ChronoCube emulator core.

#include "cc_core.h"

#include <assert.h>
#include <SDL/SDL.h>

#include "cc_internal.h"

static struct {
  uint8_t* vram;

  struct {
    uint16_t x;
    uint16_t y;
  } scroll;

  uint8_t enabled;
  uint8_t blanked;

  CC_Palette* palettes;
  uint8_t num_palettes;

  CC_TileLayer* tile_layers;
  uint8_t num_tile_layers;

  CC_Sprite* sprites;
  uint16_t num_sprites;
} cc;

// Used for rendering with SDL.
static struct {
  SDL_Surface* screen;        // The video screen.
  SDL_Surface* vram;          // VRAM with image data.
  SDL_Surface** tile_layers;  // Surfaces for drawing tile layers.
} renderer;

void CC_Init() {
  int i;

  cc.vram = malloc(VRAM_SIZE);

  cc.palettes = calloc(NUM_PALETTES, sizeof(CC_Palette));
  cc.num_palettes = NUM_PALETTES;
  for (i = 0; i < NUM_PALETTES; ++i)
    cc.palettes[i].data = calloc(NUM_COLORS_PER_PALETTE, 4);

  cc.tile_layers = calloc(NUM_TILE_LAYERS, sizeof(CC_TileLayer));
  cc.num_tile_layers = NUM_TILE_LAYERS;
  for (i = 0; i < NUM_TILE_LAYERS; ++i)
    cc.tile_layers[i].tiles = calloc(TILE_MAP_SIZE, sizeof(uint16_t));

  cc.sprites = calloc(NUM_SPRITES, sizeof(CC_Sprite));
  cc.num_sprites = NUM_SPRITES;

  cc.enabled = 0;
  cc.blanked = 0;
  cc.scroll.x = 0;
  cc.scroll.y = 0;

  CC_RendererInit();
}

void CC_Cleanup() {
  int i;

  CC_RendererCleanup();

  free(cc.vram);

  for (i = 0; i < NUM_TILE_LAYERS; ++i)
    free(cc.palettes[i].data);
  free(cc.palettes);

  for (i = 0; i < NUM_TILE_LAYERS; ++i)
    free(cc.tile_layers[i].tiles);
  free(cc.tile_layers);

  free(cc.sprites);
}

void CC_SetVramData(uint32_t vram_offset, void* src_data, uint32_t size) {
  assert(vram_offset + size <= VRAM_SIZE);
  memcpy(cc.vram + vram_offset, src_data, size);
}

void CC_SetPaletteData(uint8_t index, void* data, uint16_t size) {
  assert(index < cc.num_palettes);
  assert(size <= PALETTE_SIZE);
  memcpy(cc.palettes[index].data, data, size);
}

void CC_SetPaletteEntry(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
  assert(index < cc.num_palettes);
  CC_Palette* palette = &cc.palettes[index];
  palette->entries[index].r = r;
  palette->entries[index].g = g;
  palette->entries[index].b = b;
}

void CC_SetOutputEnable(uint8_t enabled) {
  cc.enabled = enabled;
}

void CC_SetOutputBlank(uint8_t blanked) {
  cc.blanked = blanked;
}

void CC_SetScrollOffset(uint16_t x, uint16_t y) {
  cc.scroll.x = x;
  cc.scroll.y = y;
}

CC_TileLayer* CC_GetTileLayer(uint8_t index) {
  assert(index < cc.num_tile_layers);
  return &cc.tile_layers[index];
}

CC_Sprite* CC_GetSprite(uint16_t index) {
  assert(index < cc.num_sprites);
  return &cc.sprites[index];
}

void CC_RendererInit() {
  int i;
  uint32_t rmask, gmask, bmask, amask;

  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);

  renderer.screen =
      SDL_SetVideoMode (SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
  assert(renderer.screen);
  // RGB masks for all surfaces should match the screen's.
  rmask = renderer.screen->format->Rmask;
  gmask = renderer.screen->format->Gmask;
  bmask = renderer.screen->format->Bmask;
  amask = 0;  // Turn off per-pixel alpha.

  renderer.vram = SDL_CreateRGBSurfaceFrom(
      cc.vram,
      TILE_WIDTH,
      VRAM_SIZE / TILE_WIDTH,
      8,              // TODO: make this a #define
      TILE_WIDTH,
      rmask, gmask, bmask, amask);
  assert(renderer.vram);

  renderer.tile_layers = calloc(sizeof(SDL_Surface*), cc.num_tile_layers);
  for (i = 0; i < cc.num_tile_layers; ++i) {
    renderer.tile_layers[i] = SDL_CreateRGBSurface(
        SDL_HWSURFACE | SDL_SRCALPHA | SDL_SRCCOLORKEY,
        TILE_LAYER_WIDTH,
        TILE_LAYER_HEIGHT,
        renderer.screen->format->BitsPerPixel,
        rmask, gmask, bmask, amask);
    assert(renderer.tile_layers[i]);
  }
}

void CC_RendererCleanup() {
  int i;
  for (i = 0; i < cc.num_tile_layers; ++i)
    SDL_FreeSurface(renderer.tile_layers[i]);
  SDL_FreeSurface(renderer.vram);
  SDL_Quit();
}

void CC_RendererDraw() {
  int i;
  SDL_Rect src;
  src.w = TILE_WIDTH;
  src.h = TILE_HEIGHT;
  SDL_Rect dst;
  dst.w = TILE_WIDTH;
  dst.h = TILE_HEIGHT;

  // Clear the screen.
  SDL_FillRect(renderer.screen, NULL, 0);

  // Get the RGB masks for the screen.
  uint32_t rmask = renderer.screen->format->Rmask;
  uint32_t gmask = renderer.screen->format->Gmask;
  uint32_t bmask = renderer.screen->format->Bmask;

  // Use the RGB masks to determine which bits are not being used for color.
  // They can be used to set a transparent color key that does not represent an
  // actual color value.
  uint32_t transparent_color = ~(rmask | gmask | bmask);

  for (i = 0; i < NUM_TILE_LAYERS; ++i) {
    const CC_TileLayer* tile_layer = &cc.tile_layers[i];
    if (!tile_layer->enabled)
      continue;

    // Set the rendering palette for this layer.
    const CC_Palette palette = cc.palettes[cc.tile_layers[i].palette];
    SDL_SetColors(renderer.vram,
                  (SDL_Color*)palette.data,
                  0,
                  NUM_COLORS_PER_PALETTE);

    // Render tiles onto the tile layer surface.
    SDL_Surface* layer = renderer.tile_layers[i];

    // Turn on alpha for the surface as a whole.
    uint8_t layer_alpha = tile_layer->enable_alpha ? tile_layer->alpha : 0xff;
    if (tile_layer->enable_alpha)
      SDL_SetAlpha(layer, SDL_SRCALPHA | SDL_SRCCOLORKEY, layer_alpha);

    // Set the transparent value (color key) for blitting from VRAM to layer.
    if (tile_layer->enable_trans)
      SDL_SetColorKey(renderer.vram, SDL_SRCCOLORKEY, tile_layer->trans_value);

    // Set the transparency color for blitting from layer to screen.
    if (tile_layer->enable_trans | tile_layer->enable_nop)
      SDL_SetColorKey(layer, SDL_SRCCOLORKEY, transparent_color);

    // Clear the surface.  If there is no transparency, this will just be black.
    SDL_FillRect(layer, NULL, transparent_color);

    int tile_index = 0;
    for (dst.y = 0; dst.y < TILE_LAYER_HEIGHT; dst.y += TILE_HEIGHT) {
      for (dst.x = 0; dst.x < TILE_LAYER_WIDTH; dst.x += TILE_WIDTH) {
        uint16_t tile_value = tile_layer->tiles[tile_index++] & 0x1fff;
        // Skip rendering the tile if it is a NOP tile.
        if (tile_layer->enable_nop && tile_layer->nop_value == tile_value)
          continue;
        src.x = 0;
        src.y = tile_value * TILE_HEIGHT;
        SDL_BlitSurface(renderer.vram, &src, layer, &dst);
      }
    }

    // Render the tile layer surface onto the screen.
    SDL_Rect screen_dst;
    screen_dst.x = cc.tile_layers[i].x - cc.scroll.x;
    screen_dst.y = cc.tile_layers[i].y - cc.scroll.y;

    // Handle wrap-around.
    // TODO: drawing the layer four times is inefficient.

    // Wrap horizontally.
    if (screen_dst.x + TILE_LAYER_WIDTH < SCREEN_WIDTH) {
      SDL_Rect screen_dst_wrap_x = screen_dst;
      screen_dst_wrap_x.x += TILE_LAYER_WIDTH;
      SDL_BlitSurface(layer, NULL, renderer.screen, &screen_dst_wrap_x);
    }
    // Wrap vertically.
    if (screen_dst.y + TILE_LAYER_HEIGHT < SCREEN_HEIGHT) {
      SDL_Rect screen_dst_wrap_y = screen_dst;
      screen_dst_wrap_y.y += TILE_LAYER_WIDTH;

      // Wrap diagonally.
      if (screen_dst.x + TILE_LAYER_WIDTH < SCREEN_WIDTH) {
        SDL_Rect screen_dst_wrap_xy = screen_dst_wrap_y;
        screen_dst_wrap_xy.x += TILE_LAYER_WIDTH;
        SDL_BlitSurface(layer, NULL, renderer.screen, &screen_dst_wrap_xy);
      }
      SDL_BlitSurface(layer, NULL, renderer.screen, &screen_dst_wrap_y);
    }
    SDL_BlitSurface(layer, NULL, renderer.screen, &screen_dst);
  }

  for (i = 0; i < NUM_SPRITES; ++i) {
    const CC_Sprite* sprite = &cc.sprites[i];
    if (!sprite->enabled)
      continue;

    // Allocate data for the sprite surface.
    char* surface_data = calloc(sprite->w * sprite->h, 1);

    // Copy the sprite data from VRAM to the sprite surface.
    const char* image_data = cc.vram + sprite->data_offset;
    if (!sprite->flip_x && !sprite->flip_y && !sprite->flip_xy) {
      // If no flipping enabled, just copy the memory verbatim.
      memcpy(surface_data, image_data, sprite->w * sprite->h);
    } else {
      // Copy the sprite data one pixel at a time if flipping is enabled.
      // TODO: this part is highly inefficient and error-prone.  Rewrite it.
      int x, y;
      int dst_x, dst_y;
      int dst_w = sprite->flip_xy ? sprite->h : sprite->w;
      for (y = 0; y < sprite->h; ++y) {
        for (x = 0; x < sprite->w; ++x) {
          if (sprite->flip_xy) {
            dst_x = sprite->flip_y ? y : (sprite->h - 1 - y);
            dst_y = sprite->flip_x ? x : (sprite->w - 1 - x);
          } else {
            dst_x = sprite->flip_x ? x : (sprite->w - 1 - x);
            dst_y = sprite->flip_y ? y : (sprite->h - 1 - y);
          }
          assert(dst_y * dst_w + dst_x < sprite->w * sprite->h);
          surface_data[dst_y * dst_w + dst_x] = image_data[y * sprite->w + x];
        }
      }
    }

    // Now generate the surface from the copied sprite data.
    SDL_Surface *sprite_surface =
        SDL_CreateRGBSurfaceFrom(surface_data,
                                 sprite->flip_xy ? sprite->h : sprite->w,
                                 sprite->flip_xy ? sprite->w : sprite->h,
                                 8,
                                 sprite->flip_xy ? sprite->h : sprite->w,
                                 rmask, gmask, bmask, 0);

    // Set the rendering palette for the sprite.
    const CC_Palette* palette = &cc.palettes[sprite->palette];
    SDL_SetColors(sprite_surface,
                  (SDL_Color*)palette->data,
                  0,
                  NUM_COLORS_PER_PALETTE);

    // Turn on alpha for the surface as a whole.
    if (sprite->enable_alpha) {
      SDL_SetAlpha(sprite_surface,
                   SDL_SRCALPHA | SDL_SRCCOLORKEY,
                   sprite->alpha);
    }

    // Set the transparency color for blitting from layer to screen.
    if (sprite->enable_trans)
      SDL_SetColorKey(sprite_surface, SDL_SRCCOLORKEY, sprite->trans_value);

    // Draw to the screen.
    SDL_Rect dst;
    dst.x = sprite->x - sprite->ref_x - cc.scroll.x;
    dst.y = sprite->y - sprite->ref_y - cc.scroll.y;
    dst.w = sprite->w;
    dst.h = sprite->h;
    SDL_BlitSurface(sprite_surface, NULL, renderer.screen, &dst);

    SDL_FreeSurface(sprite_surface);
    free(surface_data);
  }

  SDL_Flip(renderer.screen);
}
