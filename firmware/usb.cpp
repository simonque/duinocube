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

// DuinoCube coprocessor USB host functions.

#include <string.h>

#include <avr/io.h>

#include "defines.h"
#include "printf.h"
#include "spi.h"
#include "usb/Usb.h"
#include "usb/USBJoystick.h"

#include "usb.h"

#define UINT8_MAX     ((uint8_t)(~0))

static USBJoystick joystick;    // For USB joystick.

USB_JoystickState joystick_state;   // Last known joystick state.

const char joystick_update_stick_str0[] PROGMEM = "Updating joystick stick: ";
const char joystick_update_stick_str1[] PROGMEM = "X = %d\n";
const char joystick_update_stick_str2[] PROGMEM = "Y = %d\n";
const char joystick_update_stick_str3[] PROGMEM =
    "Unknown joystick axis changed: %u, value: %u\n";

// Callbacks for updating joystick values.
#define JOYSTICK_AXIS_X      0
#define JOYSTICK_AXIS_Y      1
static void joystick_update_stick(uint8_t axis, uint8_t value) {
  switch(axis) {
  case JOYSTICK_AXIS_X:
#ifdef DEBUG
    printf_P(joystick_update_stick_str0);
    printf_P(joystick_update_stick_str1, value);
#endif
    joystick_state.x = value;
    break;
  case JOYSTICK_AXIS_Y:
#ifdef DEBUG
    printf_P(joystick_update_stick_str0);
    printf_P(joystick_update_stick_str2, value);
#endif
    joystick_state.y = value;
    break;
  default:
#ifdef DEBUG
    printf_P(joystick_update_stick_str3, axis, value);
#endif
    break;
  }
}

const char joystick_update_button_str0[] PROGMEM =
    "Updating joystick button: [%u] = %u\n";

static void joystick_update_button(uint8_t button, uint8_t value) {
#ifdef DEBUG
  printf_P(joystick_update_button_str0, button, value);
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

const char usb_init_str0[] PROGMEM = "USB powered on.\n";

static bool usb_enabled;

void usb_init() {
  // Disable the USB select pin before setting it as an output.
  spi_clear_ss(SELECT_USB_BIT);
  DDRC |= (1 << SELECT_USB_BIT);

  // Set the pullup of the USB supported bit.
  // TODO: eventually there should be an external pullup?
  DDRD &= ~(1 << USB_ENABLED_BIT);
  PORTD |= (1 << USB_ENABLED_BIT);
  // Determine if USB is supported.
  usb_enabled = PIND & (1 << USB_ENABLED_BIT);
  if (!usb_enabled)
    return;

#ifdef DEBUG
  printf_P(usb_init_str0);
#endif

  // Set up USB joystick.

  // Clear the joystick state.
  memset(&joystick_state, 0, sizeof(joystick_state));
  // Center the cached joystick value.
  joystick_state.x = UINT8_MAX / 2;
  joystick_state.y = UINT8_MAX / 2;

  // Specify callbacks to update joystick state.
  joystick.setStickValueDidChangeCallback(joystick_update_stick);
  joystick.setButtonValueDidChangeCallback(joystick_update_button);
  joystick.setHatValueDidChangeCallback(joystick_update_hat);
  joystick.init();
}

void usb_update() {
  if (!usb_enabled)
    return;
  joystick.run();
}

void usb_read_joystick(USB_JoystickState* state) {
  *state = joystick_state;
}
