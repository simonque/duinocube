// Copyright (C) 2014 Simon Que
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

// Display functions.

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>

// Initializes text rendering using a particular layer and palette.
void display_text_init(uint8_t layer, uint8_t palette);

// Clears |length| characters at the given location.
void display_text_clear(uint8_t len, uint8_t x, uint8_t y);

// Renders text at the given location.
void display_text_render(const char* text, uint8_t x, uint8_t y);

// Renders text at the given location from program memory.
void display_text_render_P(const char* text, uint8_t x, uint8_t y);

#endif  // __DISPLAY_H__
