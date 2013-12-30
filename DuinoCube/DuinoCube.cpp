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

// DuinoCube library for Arduino.

#include <stdio.h>

#include <Arduino.h>
#include <SPI.h>

#if defined(__AVR_ATmega32U4__)
#include <Esplora.h>
#endif  // defined(__AVR_ATmega32U4__)

#include "DuinoCube.h"

// Select pins for Core and System shields.
// Edit these to match your Arduino configuration.
#define DEFAULT_CORE_SS_PIN      5
#define DEFAULT_SYS_SS_PIN      10

extern SPIClass SPI;

DuinoCubeCore DuinoCube::Core;
DuinoCubeFile DuinoCube::File;
DuinoCubeSystem DuinoCube::Sys;

static FILE uart_stdout;  // For linking UART to printf, etc.
static int uart_putchar (char c, FILE *stream) {
  Serial.write(c);
  return 0;
}

void DuinoCube::begin() {
  // Initialize SPI.
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);

  // Set up printf over UART.
  fdev_setup_stream(&uart_stdout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uart_stdout;

  // On Arduino Leonardo / Esplora, the system doesn't wait for the serial
  // console to be opened. Important messages may be lost. Wait for console
  // input manually to continue. Alternatively, press a button on the Esplora
  // to continue.
#if defined(__AVR_ATmega32U4__)
  while (!Serial.available() &&
         Esplora.readButton(SWITCH_1) == HIGH &&
         Esplora.readButton(SWITCH_2) == HIGH &&
         Esplora.readButton(SWITCH_3) == HIGH &&
         Esplora.readButton(SWITCH_4) == HIGH);
#endif  // defined(__AVR_ATmega32U4__)

  Core.begin();
  Sys.begin();
}
