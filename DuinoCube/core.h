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

// DuinoCube library for the DuinoCube audio/video core.

#ifndef __DUINOCUBE_CORE_H__
#define __DUINOCUBE_CORE_H__

#include <stdint.h>

#include "core_defs.h"

class DuinoCubeCore {
 public:
  // Initialize and teardown functions.
  static void begin();

  // System control functions.
  static void moveCamera(int16_t x, int16_t y);
  static void waitForEvent(unt16_t event);

  // Data loading functions.
  static bool loadPalette(const char* filename, uint8_t palette_index);
  static bool loadTilemap(const char* filename, uint8_t tilemap_index);
  static uint32_t loadImageData(const char* filename, uint32_t vram_offset);

  // Tile layer functions.
  static void enableTileLayer(uint8_t layer_index);
  static void disableTileLayer(uint8_t layer_index);
  static void moveTileLayer(uint8_t layer_index, int16_t x, int16_t y);
  static void setTileLayerProperty(uint8_t layer_index, uint16_t property,
                                   uint16_t value);

  // Sprite functions.
  static void enableSprite(uint8_t sprite_index);
  static void disableSprite(uint8_t sprite_index);
  static void moveSprite(uint8_t sprite_index, int16_t x, int16_t y);
  static void setSpriteProperty(uint8_t sprite_index, uint16_t property,
                                uint16_t value);

  // TODO: the preceding functions are higher level functions than the
  // memory-level access functions that follow. The former should replace the
  // latter as Core API. The below functions should become private.

  // Functions to read/write bytes and words.
  static uint8_t readByte(uint16_t addr);
  static void writeByte(uint16_t addr, uint8_t data);
  static uint16_t readWord(uint16_t addr);
  static void writeWord(uint16_t addr, uint16_t data);

  // Functions to read/write block data.
  static void readData(uint16_t addr, void* data, uint16_t size);
  static void writeData(uint16_t addr, const void* data, uint16_t size);
};

#endif  // __DUINOCUBE_CORE_H__
