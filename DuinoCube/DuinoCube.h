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

// Main DuinoCube library for Arduino.

#ifndef __DUINOCUBE_H__
#define __DUINOCUBE_H__

#include "DuinoCube_core.h"
#include "DuinoCube_file.h"
#include "DuinoCube_gamepad.h"
#include "DuinoCube_mem.h"
#include "DuinoCube_rpc.h"
#include "DuinoCube_system.h"
#include "DuinoCube_usb.h"

class DuinoCube {
 public:
  // Initializes all subsystems.
  static void begin();

  // Objects for accessing subsystems.
  static DuinoCubeCore Core;
  static DuinoCubeFile File;
  static DuinoCubeGamepad Gamepad;
  static DuinoCubeMemory Mem;
  static DuinoCubeRPC RPC;
  static DuinoCubeSystem Sys;
  static DuinoCubeUSB USB;
};

extern DuinoCube DC;

#endif  // __DUINOCUBE_H__
