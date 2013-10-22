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

// DuinoCube library for Arduino.

#include <stdio.h>

#include <Arduino.h>
#include <SPI.h>

#include "DuinoCube.h"

// Select pins for Core and System shields.
// Edit these to match your Arduino configuration.
#define DEFAULT_CORE_SS_PIN      5
#define DEFAULT_SYS_SS_PIN       4

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

  Core.begin(DEFAULT_CORE_SS_PIN);
  Sys.begin(DEFAULT_SYS_SS_PIN);
}
