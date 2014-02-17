// Copyright (C) 2014 Simon Que
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

// Boot controller.

#ifndef __BOOT_H__
#define __BOOT_H__

// Determines if the system is in boot mode or dev mode.
// Dev mode: Arduino controls and resets coprocessor.
// Boot mode: Coprocessor acts as a boot controller for Arduino.
bool boot_mode_enabled();

// Run the boot controller.
void boot_run();

#endif  // __BOOT_H__
