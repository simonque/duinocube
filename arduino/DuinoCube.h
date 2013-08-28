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

// DuinoCube library for Arduino.

#ifndef __DUINOCUBE_H__
#define __DUINOCUBE_H__

#include <stdint.h>

#include "DuinoCube_defs.h"
#include "DuinoCube_rpc.h"

class DuinoCube {
 public:
  // Initialize and teardown functions.
  static void begin();
  static void begin(uint8_t ss_pin, uint8_t sys_ss_pin);
  static void end();

  // Functions to read/write bytes and words.
  static uint8_t readByte(uint16_t addr);
  static void writeByte(uint16_t addr, uint8_t data);
  static uint16_t readWord(uint16_t addr);
  static void writeWord(uint16_t addr, uint16_t data);

  // Functions to read/write block data.
  static void readData(uint16_t addr, void* data, uint16_t size);
  static void writeData(uint16_t addr, const void* data, uint16_t size);

  // Resets the coprocessor.
  static void resetRPCServer();

  // RPC functions: see DuinoCube_rpc for descriptions.
  // All functions return a status.
  static int rpcHello();
  static int rpcInvert(uint16_t buf, uint16_t size);

 private:
  // Writes a byte to the RPC status register.
  static void writeRPCCommandStatus(uint8_t value);
  // Writes a byte to the coprocessor status register.
  static uint8_t readRPCServerStatus();

  // TODO: add serial RAM access functions.

  static uint8_t s_ss_pin;      // Pin for selecting DuinoCube Core Shield.
  static uint8_t s_sys_ss_pin;  // Pin for selecting DuinoCube System Shield.
};

extern DuinoCube DC;

#endif  // __DUINOCUBE_H__
