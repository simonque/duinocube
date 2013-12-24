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

#include "DuinoCube_gamepad.h"
#include "DuinoCube_rpc.h"
#include "DuinoCube_rpc_usb.h"

#include "DuinoCube_usb.h"

#ifndef NULL
#define NULL              0
#endif

static DuinoCubeRPC rpc;

GamepadState DuinoCubeUSB::readJoystick() {
  GamepadState state;

  RPC_UsbReadJoystickArgs args;
  rpc.exec(RPC_CMD_USB_READ_JOYSTICK, NULL, 0, &args.out, sizeof(args.out));

  state.buttons = args.out.buttons;
  state.x = args.out.x;
  state.y = args.out.y;
  return state;
}
