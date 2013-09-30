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

// DuinoCube remote procedure call functions for USB host.

#include "DuinoCube_rpc.h"
#include "DuinoCube_rpc_usb.h"

#include "shmem.h"
#include "usb.h"

#include "rpc_usb.h"

void rpc_usb_read_joystick() {
  USB_JoystickState state;
  usb_read_joystick(&state);

  RPC_UsbReadJoystickArgs args;
  args.out.buttons = state.buttons;
  args.out.x = state.x;
  args.out.y = state.y;

  shmem_write(OUTPUT_ARG_ADDR, &args.out, sizeof(args.out));
}
