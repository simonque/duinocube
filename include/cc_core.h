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

// ChronoCube main API.

#ifndef _CC_CORE_H_
#define _CC_CORE_H_

#include <stdint.h>

#include "cc_sprite.h"
#include "cc_tile_layer.h"

// Setup and cleanup functions for ChronoCube.
void CC_Init();
void CC_Cleanup();

// Bulk copy raw data to VRAM.
void CC_SetVramData(uint32_t vram_offset, void* src_data, uint32_t size);

// Bulk copy palette data, with each entry packed to four bytes.
void CC_SetPaletteData(uint8_t index, void* data, uint16_t size);

// Set one palette entry in the palette indicated by |index|.
void CC_SetPaletteEntry(uint8_t index, uint8_t r, uint8_t g,uint8_t b);

// Turn video output on/off.
void CC_SetOutputEnable(uint8_t enabled);

// Blank video image.
void CC_SetOutputBlank(uint8_t blanked);

// Set scrolling offset, relative to world space.
void CC_SetScrollOffset(uint16_t x, uint16_t y);

// These are for the emulator renderer.
// TODO: put in separate file.

// Initialize renderer resources.
void CC_RendererInit();

// Free renderer resources.
void CC_RendererCleanup();

// Main ChronoCube emulator function.  Draws to screen.
void CC_RendererDraw();

#endif  // _CC_CORE_H_
