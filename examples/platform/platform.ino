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

// A platform game demo.

#include <DuinoCube.h>
#include <SPI.h>

#if defined(__AVR_Atmega32U4__)
#include <Esplora.h>
#endif

#include "resources.h"

extern uint8_t __bss_end;   // End of statically allocated memory.
extern uint8_t __stack;     // Where local variables are allocated.

void setup() {
  Serial.begin(115200);
  DC.begin();

  loadResources();

  setupLayers();

  printf("Static data ends at 0x%04x\n", &__bss_end);
  printf("Stack is at 0x%04x\n", &__stack);

  // Initialize random generator with time.
  // TODO: Use a less deterministic seed value.
  srand(millis());
}

void loop() {
}
