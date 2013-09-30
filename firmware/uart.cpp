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

// DuinoCube coprocessor UART functions.

#include <stdio.h>

#include <avr/io.h>

#define FOSC       20000000    // System runs at 20 MHz.
#define BAUD         230400    // Desired UART baud rate.
// This UBRR register value gets baud rate |BAUD| given clock frequency |FOSC|.
// It is obtained from Table 19-12 of the Atmega328P datasheet.
#define UBRR_VAL         10

static FILE mystdout;
static FILE mystdin;

// For initializing stream objects (FILEs).
static void init_fdev(FILE* stream,
            int (*put_func)(char, FILE*),
            int (*get_func)(FILE*),
            int flags) {
  stream->flags = flags;
  stream->put = put_func;
  stream->get = get_func;
  stream->udata = NULL;
}

// Write a byte over UART.
static int uart_putchar(char c, FILE *stream) {
  if (c == '\n')
  uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;

  return 0;
}

// Read a byte over UART.
static int uart_getchar(FILE *stream) {
  while( !(UCSR0A & (1 << RXC0)) );
  return(UDR0);
}

// Initializes AVR UART and Standard I/O.
void uart_init() {
  UBRR0H = UBRR_VAL >> 8;
  UBRR0L = UBRR_VAL;
  UCSR0A = (1 << U2X0);
  UCSR0B = (1 << TXEN0);
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
  DDRD = (1 << PORTD1);

  // Set up stdio streams to use UART.
  init_fdev(&mystdout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  init_fdev(&mystdin, NULL, uart_getchar, _FDEV_SETUP_READ);

  stdout = &mystdout;  // Required for printf over UART.
  stderr = &mystdout;  // Required for fprintf(stderr) over UART.
  stdin = &mystdin;    // Required for scanf over UART.
}
