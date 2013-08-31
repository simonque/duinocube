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

// DuinoCube file system interface.

#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "fatfs/diskio.h"
#include "fatfs/ff.h"
#include "spi.h"

#include "file.h"

// File handles are statically allocated.  This is the max number of handles.
#define MAX_NUM_FILE_HANDLES    4

// The basic FatFS object for file system operations.
static FATFS fatfs;

static FIL file_handles[MAX_NUM_FILE_HANDLES];
static uint8_t file_handle_active[MAX_NUM_FILE_HANDLES];

void file_init() {
  // Enable the 10 ms timer for FatFS.
  TCCR0B |= (1 << CS02) | (1 << CS00);  // speed = F_CPU / 1024.
  OCR0A = 200;                          // 10 ms interrupt at 20 MHz.
  TIMSK0 |= (1 << OCIE0A);    // Enable interrupt for timer match A.

  sei();    // Enable interrupts.

#ifdef DEBUG
  printf("SD card timer initialized\n");
#endif

  // Clear the file handle states.
  for (int i = 0; i < MAX_NUM_FILE_HANDLES; ++i)
    file_handle_active[i] = false;

  // Initialize the SD card file system.
  disk_initialize(0);
  f_mount(0, &fatfs);
}

// TODO: return more descriptive status values.

int file_open(const char* filename, uint8_t mode, uint16_t* handle) {
  // Get the first free file handle.
  uint16_t i = 0;
  while (i < MAX_NUM_FILE_HANDLES && file_handle_active[i])
    ++i;
  if (i == MAX_NUM_FILE_HANDLES)
    return 1;
  *handle = i;

  // Open using the free file handle, if it was found.
  if (f_open(&file_handles[*handle], filename, mode))
    return 1;

  file_handle_active[*handle] = true;
  return 0;
}

int file_close(uint16_t handle) {
  // Do not close an invalid file handle.
  if (handle >= MAX_NUM_FILE_HANDLES)
    return 1;

  if (f_close(&file_handles[handle]))
    return 1;
  file_handle_active[handle] = false;;
  return 0;
}

int file_read(uint16_t handle, void* dst, uint16_t size) {
  if (handle >= MAX_NUM_FILE_HANDLES)
    return 1;

  UINT size_read;
  if (f_read(&file_handles[handle], dst, size, &size_read))
    return 1;

  return (size_read != size);
}

int file_write(uint16_t handle, const void* src, uint16_t size) {
  if (handle >= MAX_NUM_FILE_HANDLES)
    return 1;

  UINT size_written;
  if (f_write(&file_handles[handle], src, size, &size_written))
    return 1;

  return (size_written != size);
}

// Timer interrupt handler for file system.
ISR(TIMER0_COMPA_vect) {
  disk_timerproc();
}
