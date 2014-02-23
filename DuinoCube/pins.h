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

// DuinoCube pin assignments.

#ifndef __DUINOCUBE_PINS_H__
#define __DUINOCUBE_PINS_H__

#include <Arduino.h>

#if defined(ARDUINO)

  // For Arduino Mega.
  #if defined(__AVR_ATmega1280__) || (__AVR_ATmega2560__)
    #error "Arduinos with Atmega1280 and Atmega2560 not yet supported."

  // For Arduino Uno.
  #elif defined(__AVR_ATmega328P__)

    // Active low, indicates client issued a command.
    #define RPC_CLIENT_COMMAND_PIN  PORTD, 3
    #define RPC_CLIENT_COMMAND_DIR  DDRD, 3
    // Active low, indicates server is busy.
    #define RPC_SERVER_STATUS_PIN   PIND, 4
    #define RPC_SERVER_STATUS_DIR   DDRD, 4
    // Active low, resets the coprocessor.
    #define RPC_RESET_PIN           PORTD, 7
    #define RPC_RESET_DIR           DDRD, 7

    // Active low, selects the system shield SPI RAM.
    #define RAM_SELECT_PIN          PORTB, 2
    #define RAM_SELECT_DIR          DDRB, 2

    // Active low, selects the core shield.
    #define CORE_SELECT_PIN         PORTD, 5
    #define CORE_SELECT_DIR         DDRD, 5

  // For Arduino Esplora.
  #elif defined(__AVR_ATmega32U4__)
    // Active low, indicates client issued a command.
    #define RPC_CLIENT_COMMAND_PIN  PORTD, 2
    #define RPC_CLIENT_COMMAND_DIR  DDRD, 2
    // Active low, indicates server is busy.
    #define RPC_SERVER_STATUS_PIN   PINB, 4
    #define RPC_SERVER_STATUS_DIR   DDRB, 4
    // Active low, resets the coprocessor.
    #define RPC_RESET_PIN           PORTD, 3
    #define RPC_RESET_DIR           DDRD, 3

    // Active low, selects the system shield SPI RAM.
    #define RAM_SELECT_PIN          PORTE, 6
    #define RAM_SELECT_DIR          DDRE, 6

    // Active low, selects the core shield.
    #define CORE_SELECT_PIN         PORTD, 0
    #define CORE_SELECT_DIR         DDRD, 0

  #endif

#else
  #error "Non-Arduino systems not yet supported."
#endif  // defined(ARDUINO)

// Macro to set and read a bit in a word.
#define SET_BIT(word, bit, value) do { \
    if (value) \
      ((word) |= (1 << (bit))); \
    else \
      ((word) &= ~(1 << (bit))); \
  } while (false);
#define GET_BIT(word, bit) (((word) >> bit) & 1)

// Macro to set a port pin. It uses the above defines that contain a port and
// pin number.
#define SET_PIN(port_and_pin, value) SET_BIT(port_and_pin, value)

// Read a pin value.
#define GET_PIN(port_and_pin) GET_BIT(port_and_pin)

#endif  // __DUINOCUBE_PINS_H__
