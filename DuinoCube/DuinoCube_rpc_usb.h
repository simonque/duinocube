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

// DuinoCube Remote Procedure Call (RPC) definitions: USB operations.

#ifndef __DUINOCUBE_RPC_USB_H__
#define __DUINOCUBE_RPC_USB_H__

#include <stdint.h>

// Note: all fields are to be at least 16-bit ints for alignment compatibility
// between 8-bit and non-8-bit systems (e.g. ARM).

// TODO: implement more basic USB host commands.

struct RPC_UsbReadJoystickArgs {
  // No inputs.
  struct {
    uint16_t buttons;       // Each set bit represents a button that is pressed.
    int16_t x;              // Joystick direction along X and Y axes.
    int16_t y;
  } out;
};

#endif  // __DUINOCUBE_RPC_USB_H__
