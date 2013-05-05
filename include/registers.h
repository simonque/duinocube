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

// ChronoCube tile layer register definitions.

#ifndef _REGISTERS_H_
#define _REGISTERS_H_

// These are not final register indexes, and will be updated as the ChronoCube
// hardware changes.
#define CC_REG_ID               0
#define CC_REG_OUTPUT_STATUS    1
#define CC_REG_SCAN_X           2
#define CC_REG_SCAN_Y           3

#define CC_REG_MEM_CTRL         5

#define CC_REG_SPRITE_Z         7
#define CC_REG_SCROLL_X        14
#define CC_REG_SCROLL_Y        15

// Output status register.
#define CC_REG_HSYNC            0
#define CC_REG_VSYNC            1
#define CC_REG_HBLANK           2
#define CC_REG_VBLANK           3

// Memory control register.
#define CC_REG_ENABLE_VRAM_OFFSET     0
#define CC_REG_BANK_OFFSET            8

#endif  // _REGISTERS_H_
