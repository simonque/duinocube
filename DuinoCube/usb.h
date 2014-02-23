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

// DuinoCube USB library for Arduino.

#ifndef __DUINOCUBE_USB_H__
#define __DUINOCUBE_USB_H__

#include <stdint.h>

struct GamepadState;  // Struct containing gamepad state data.

namespace DuinoCube {

class USB {
 public:
  // Returns joystick button/stick values.
  static GamepadState readJoystick();
};

}  // namespace DuinoCube

#endif  // __DUINOCUBE_USB_H__
