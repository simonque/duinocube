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

// Internal timer system.

#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "diskio.h"

#include "timer.h"

static uint16_t ms_counter;

static uint8_t interrupts_enabled = 0;

void timer_init() {
  // Enable ~10 ms timer for FatFS.
  TCCR0B |= (1 << CS02) | (1 << CS00);  // speed = F_CPU / 1024.
  OCR0A = 200;                          // 10 ms interrupt at 20 MHz.
  TIMSK0 |= (1 << OCIE0A);              // Enable interrupt for timer match A.

  // Enable millsecond timer for general use.
  TCCR1B |= (1 << WGM12);               // Configure Timer 1 for CTC mode.
  TCCR1B |= (1 << CS11);                // speed = F_CPU / 8.
  TIMSK1 |= (1 << OCIE1A);              // Enable CTC interrupt.
  OCR1A   = 2500;        // Set CTC compare value to trigger at 1 kHz given
                         // a 20-MHz clock with prescaler of 8 (= 2.5 MHz).

  timer_reset();

  interrupts_enabled = 1;
  sei();                 // Enable global interrupts.
}

void timer_reset() {
  uint8_t reenable_interrupts = 0;
  if (interrupts_enabled) {
    cli();
    interrupts_enabled = 0;
    reenable_interrupts = 1;
  }

  ms_counter = 0;

  if (reenable_interrupts) {
    interrupts_enabled = 1;
    sei();
  }
}

uint16_t timer_get_ms() {
  uint8_t reenable_interrupts = 0;
  if (interrupts_enabled) {
    cli();
    interrupts_enabled = 0;
    reenable_interrupts = 1;
  }

  uint16_t count = ms_counter;

  if (reenable_interrupts) {
    interrupts_enabled = 1;
    sei();
  }

  return count;
}

// Interrupt handler for FatFS.
ISR(TIMER0_COMPA_vect) {
  disk_timerproc();
}

// Interrupt handler for general timer.
ISR(TIMER1_COMPA_vect) {
  ++ms_counter;
}
