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

// Font system.

// Font dimensions / size.
#define FONT_CHAR_WIDTH          8
#define FONT_CHAR_HEIGHT         8
#define FONT_CHAR_SIZE           (FONT_CHAR_WIDTH * FONT_CHAR_HEIGHT)

#define MAX_FONT_CHARS     128      // Font supports characters from 0-127.

// Loads the bitmap for the given character into the buffer.
void font_load_bitmap(char ch, void* data);
