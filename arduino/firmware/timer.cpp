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

// Internal timer system.

#include <avr/io.h>
#include <avr/interrupt.h>

#include "diskio.h"

#include "timer.h"

static uint16_t ms_counter;

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

  sei();                 // Enable global interrupts.
}

void timer_reset() {
  ms_counter = 0;
}

uint16_t timer_get_ms() {
  return ms_counter;
}

// Interrupt handler for FatFS.
ISR(TIMER0_COMPA_vect) {
  disk_timerproc();
}

// Interrupt handler for general timer.
ISR(TIMER1_COMPA_vect) {
  ++ms_counter;
}
