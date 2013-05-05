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

// AVR system initialization code.

#include <stdio.h>

#include <avr/io.h>

#define FOSC        8000000
#define BAUD          57600
// This UBRR register value gets baud rate |BAUD| given clock frequency |FOSC|.
// It is obtained from Table 20-11 of the Atmega128A datasheet.
#define UBRR_VAL         16

static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);

static void init_fdev(FILE* stream,
                      int (*put_func)(char, FILE*),
                      int (*get_func)(FILE*),
                      int flags) {
    stream->flags = flags;
    stream->put = put_func;
    stream->get = get_func;
    stream->udata = NULL;
}

static FILE mystdout;
static FILE mystdin;

static int uart_putchar(char c, FILE *stream) {
    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;

    return 0;
}

static int uart_getchar(FILE *stream) {
    while( !(UCSR0A & (1 << RXC0)) );
    return(UDR0);
}

// Initializes AVR UART.
static void init_uart() {
    UBRR0H = UBRR_VAL >> 8;
    UBRR0L = UBRR_VAL;
    UCSR0A = (1 << U2X0);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
    DDRE = (1 << PORTE1);

    stdout = &mystdout;  // Required for printf over UART.
    stderr = &mystdout;  // Required for fprintf(stderr) over UART.
    stdin = &mystdin;    // Required for scanf over UART.
}

// Initializes AVR external memory.
static void init_external_memory() {
    MCUCR = (1 << SRE);
    XMCRB = (1 << XMBK) | (1 << XMM0);
    DDRC = 0xff;
    PORTC = 0;
}

void system_init() {
    init_fdev(&mystdout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    init_fdev(&mystdin, NULL, uart_getchar, _FDEV_SETUP_READ);
    init_uart();
    init_external_memory();
}
