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

// DuinoCube coprocessor definitions.

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <avr/io.h>

// Port B bit definitions.
#define RPC_COMMAND_BIT            PORTB0
#define RPC_STATUS_BIT             PORTB1

// Port C bit definitions.
#define SELECT_RAM_BIT             PORTC0
#define SELECT_SD_BIT              PORTC1
#define SELECT_USB_BIT             PORTC2
#define SELECT_CORE_BIT            PORTC3
#define SELECT_FLASH_BIT           PORTC4
#define TEST_BIT                   PORTC5

// Port D bit definitions.
#define USB_ENABLED_BIT            PORTD7

#endif  // __DEFINES_H__
