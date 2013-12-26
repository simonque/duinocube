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

#include "DuinoCube_gamepad.h"

#if defined(__AVR_ATmega328P__)
  #include "DuinoCube_usb.h"

#elif defined(__AVR_ATmega32U4__)
  #include <Esplora.h>

#elif defined(__AVR_ATmega1280__) || (__AVR_ATmega2560__)
  #error "Arduinos with Atmega1280 and Atmega2560 not yet supported."

#endif

#define ESPLORA_JOYSTICK_MAX     512
#define ESPLORA_JOYSTICK_MIN     (-512)

// Normalizes Esplora joystick value to the range [0, UINT8_MAX].
static int GetAdjustedEsploraJoystickValue(int raw_value) {
  // Neutral position.
  if (raw_value <= ESPLORA_JOYSTICK_MAX / 2 &&
      raw_value >= ESPLORA_JOYSTICK_MIN / 2)
    return UINT8_MAX / 2;

  // Moved in one direction.
  else if (raw_value > ESPLORA_JOYSTICK_MAX / 2)
    return UINT8_MAX;

  // Moved in the other direction (ESPLORA_JOYSTICK_MIN / 2 < 0).
  else
    return 0;
}

GamepadState DuinoCubeGamepad::readGamepad() {
  GamepadState state;
#if defined(__AVR_ATmega328P__)
  DuinoCubeUSB usb;
  state = usb.readJoystick();

#elif defined(__AVR_ATmega32U4__)
  // Read the button values.
  state.buttons = 0;
  state.buttons |= (!Esplora.readButton(SWITCH_1)) << GAMEPAD_BUTTON_1;
  state.buttons |= (!Esplora.readButton(SWITCH_2)) << GAMEPAD_BUTTON_2;
  state.buttons |= (!Esplora.readButton(SWITCH_3)) << GAMEPAD_BUTTON_3;
  state.buttons |= (!Esplora.readButton(SWITCH_4)) << GAMEPAD_BUTTON_4;
  // TODO: Create an enum for the Esplora joystick button.
  state.buttons |= (!Esplora.readJoystickButton()) << GAMEPAD_BUTTON_L1;

  // For some reason, the Esplora joystick is negative on the right and positive
  // on the left. It needs to be inverted.
  state.x = GetAdjustedEsploraJoystickValue(-Esplora.readJoystickX());
  state.y = GetAdjustedEsploraJoystickValue(Esplora.readJoystickY());
  return state;

#elif defined(__AVR_ATmega1280__) || (__AVR_ATmega2560__)
  #error "Arduinos with Atmega1280 and Atmega2560 not yet supported."

#endif
  return state;
}
