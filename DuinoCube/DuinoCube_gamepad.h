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

// DuinoCube Gamepad library for Arduino.

#ifndef __DUINOCUBE_GAMEPAD_H__
#define __DUINOCUBE_GAMEPAD_H__

#include <stdint.h>

#ifndef UINT8_MAX
#define UINT8_MAX     ((uint8_t)(~0))
#endif

// TODO: These are the seemingly arbitrary mappings obtained from USB gamepad
// testing. They should be investigated at some point.
#define GAMEPAD_BUTTON_1    5
#define GAMEPAD_BUTTON_2    6
#define GAMEPAD_BUTTON_3    7
#define GAMEPAD_BUTTON_4    8
#define GAMEPAD_BUTTON_L1   9
#define GAMEPAD_BUTTON_R1  10
#define GAMEPAD_BUTTON_L2  11
#define GAMEPAD_BUTTON_R2  12

struct GamepadState {
  uint16_t buttons;     // Button states, one bit set for each button pressed.
  int16_t x, y;         // Position of stick.
};

class DuinoCubeGamepad {
 public:
  // Returns gamepad button/stick values.
  static GamepadState readGamepad();
};

#endif  // __DUINOCUBE_GAMEPAD_H__
