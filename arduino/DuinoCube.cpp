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

void DuinoCube::begin() {
  // Initialize SPI.
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);

  Core.begin(DEFAULT_CORE_SS_PIN);
  Sys.begin(DEFAULT_SYS_SS_PIN);
}
