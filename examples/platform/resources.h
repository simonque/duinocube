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

// Resource file definitions.

#ifndef __RESOURCES_H_
#define __RESOURCES_H_

#include <stdint.h>

#include "defines.h"

// VRAM offsets of background and sprite image data.
extern uint16_t g_bg_offset;
extern uint16_t g_moon_offset;
extern uint16_t g_level_offset;
extern uint16_t g_bat_offset;
extern uint16_t g_level_buffer;

// Load image, palette, and tilemap data from file system.
void loadResources();

// Initialize tile layers in DuinoCube core.
void setupLayers();

#endif  // __RESOURCES_H_
