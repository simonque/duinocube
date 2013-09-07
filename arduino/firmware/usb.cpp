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

// DuinoCube coprocessor USB host functions.

#include <stdio.h>
#include <string.h>

#include <avr/io.h>

#include "usb/Usb.h"
#include "usb/USBJoystick.h"

#include "usb.h"

static USB usb;                 // For USB host.
static USBJoystick joystick;    // For USB joystick.

USB_JoystickState joystick_state;   // Last known joystick state.

// Callbacks for updating joystick values.
#define JOYSTICK_AXIS_X      0
#define JOYSTICK_AXIS_Y      1
static void joystick_update_stick(uint8_t axis, uint8_t value) {
  switch(axis) {
  case JOYSTICK_AXIS_X:
#ifdef DEBUG
    printf("Updating joystick stick: X = %d\n", value);
#endif
    joystick_state.dx = value;
    break;
  case JOYSTICK_AXIS_Y:
#ifdef DEBUG
    printf("Updating joystick stick: Y = %d\n", value);
#endif
    joystick_state.dx = value;
    break;
  default:
#ifdef DEBUG
    printf("Unknown joystick axis changed: %u, value: %u\n", axis, value);
#endif
    break;
  }
}

static void joystick_update_button(uint8_t button, uint8_t value) {
#ifdef DEBUG
  printf("Updating joystick button: [%u] = %u\n", button, value);
#endif
  // Set or clear the corresponding button bit in the current state.
  if (value)
    joystick_state.buttons |= (1 << button);
  else
    joystick_state.buttons &= ~(1 << button);
}

static void joystick_update_hat(uint8_t hat, uint8_t value) {
  // TODO: Support joystick hat.
}

void usb_init() {
  usb.powerOn();

#ifdef DEBUG
  printf("USB powered on.\n");
#endif

  // Set up USB joystick.

  // Clear the joystick state.
  memset(&joystick_state, 0, sizeof(joystick_state));

  // Specify callbacks to update joystick state.
  joystick.setStickValueDidChangeCallback(joystick_update_stick);
  joystick.setButtonValueDidChangeCallback(joystick_update_button);
  joystick.setHatValueDidChangeCallback(joystick_update_hat);
  joystick.init();
}

void usb_update() {
  joystick.run();
}

void usb_read_joystick(USB_JoystickState* state) {
  *state = joystick_state;
}
