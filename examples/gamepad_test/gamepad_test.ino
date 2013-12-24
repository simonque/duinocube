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

// Test gamepad input.

#include <DuinoCube.h>
#include <SPI.h>

#if defined(__AVR_ATmega32U4__)
#include <Esplora.h>
#endif

static GamepadState prev_state;

void setup() {
  // Set up standard output over UART.
  Serial.begin(115200);

  DC.begin();
  memset(&prev_state, 0, sizeof(prev_state));
}

void loop() {
  // Repeatedly read in the gamepad state.
  GamepadState state = DC.Gamepad.readGamepad();
  if (memcmp(&state, &prev_state, sizeof(state)) == 0)
    return;

  // If the joystic state changed, print the new state.
  prev_state = state;
  printf("Gamepad: buttons = 0x%04x, X = %d, Y = %d\n",
         state.buttons, state.x, state.y);
}
