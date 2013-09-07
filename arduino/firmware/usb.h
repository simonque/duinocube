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

// DuinoCube coprocessor USB host module header.

#ifndef __USB_H__
#define __USB_H__

#include <stdint.h>

struct USB_JoystickState {
  uint16_t buttons;     // Each bit represents the state of a joystick button.
                        // pressed = 1, released = 0.
  int16_t dx, dy;       // Joystick axis values.
};

// Initializes the USB host.
void usb_init();

// Run the USB host task routine.
void usb_update();

// Read USB joystick position and button states.
void usb_read_joystick(USB_JoystickState* state);

#endif  // __USB_H__
