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

// Tests display timing.

#include <DuinoCube.h>
#include <SPI.h>

static FILE uart_stdout;  // For linking UART to printf, etc.
static int uart_putchar (char c, FILE *stream) {
  Serial.write(c);
  return 0;
}

void setup() {
  Serial.begin(115200);
  fdev_setup_stream(&uart_stdout, uart_putchar, NULL,
  _FDEV_SETUP_WRITE);
  stdout = &uart_stdout;

  DC.begin();
}

void loop() {
  static uint16_t t0 = millis();

  // Wait for rising edge of Vblank.
  while ((DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));
  while (!(DC.Core.readWord(REG_OUTPUT_STATUS) & (1 << REG_VBLANK)));

  uint16_t t1 = millis();

  printf("Display time: %u ms\n", t1 - t0);

  // Save the time value for the next cycle.
  t0 = t1;
}
