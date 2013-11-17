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

// DuinoCube file system interface.

#include <avr/io.h>
#include <avr/interrupt.h>

#include "defines.h"
#include "fatfs/diskio.h"
#include "fatfs/ff.h"
#include "printf.h"
#include "spi.h"

#include "file.h"

// The basic FatFS object for file system operations.
static FATFS fatfs;

// File handles are statically allocated.  This is the max number of handles.
#define MAX_NUM_FILE_HANDLES    4
static FIL file_handles[MAX_NUM_FILE_HANDLES];
static uint8_t file_handle_active[MAX_NUM_FILE_HANDLES];

// Determines if |handle| is one of the valid file handles.
// If so, returns the handle index.  Otherwise returns -1.
static int get_handle_index(const FIL* handle) {
  uint16_t index = ((uint16_t) handle - (uint16_t) file_handles) / sizeof(FIL);
  if (index < MAX_NUM_FILE_HANDLES && file_handle_active[index])
    return index;
  return -1;
}

const char file_init_str0[] PROGMEM =
    "Unable to initialize file system. disk_initialize() returned status %d\n";
const char file_init_str1[] PROGMEM =
    "Unable to mount file system. f_mount() returned status %d.\n";

void file_init() {
  // Set up the SD card SS pin.
  DDRC |= (1 << SELECT_SD_BIT);
  spi_clear_ss(SELECT_SD_BIT);

  // Clear the file handle states.
  for (int i = 0; i < MAX_NUM_FILE_HANDLES; ++i)
    file_handle_active[i] = false;

  // Initialize the SD card file system.
  int status = disk_initialize(0);
#ifdef DEBUG
  if (status != RES_OK) {
    fprintf_P(stderr, file_init_str0, status);
    return;
  }
#endif  // defined(DEBUG)

  status = f_mount(0, &fatfs);
#ifdef DEBUG
  if (status != FR_OK) {
    fprintf_P(stderr, file_init_str1, status);
  }
#endif  // defined(DEBUG)
}

const char file_open_str0[] PROGMEM = "f_open(%s, 0x%x) returned %d.\n";

uint16_t file_open(const char* filename, uint16_t mode) {
  // Get the first free file handle.
  uint16_t index = 0;
  while (index < MAX_NUM_FILE_HANDLES && file_handle_active[index])
    ++index;
  if (index == MAX_NUM_FILE_HANDLES)
    return (uint16_t) NULL;

  // Open using the free file handle, if it was found.
  int status = f_open(&file_handles[index], filename, mode);
  if (status != FR_OK) {
#ifdef DEBUG
    fprintf_P(stderr, file_open_str0, filename, mode, status);
#endif  // defined(DEBUG)
    return (uint16_t) NULL;
  }

  file_handle_active[index] = true;
  return (uint16_t) &file_handles[index];
}

void file_close(uint16_t handle) {
  FIL* file = (FIL*) handle;
  int index = get_handle_index(file);
  if (index >= 0 && file_handle_active[index] && f_close(file) == FR_OK)
    file_handle_active[index] = false;
}

const char file_read_str0[] PROGMEM =
    "f_read() of %u bytes returned status %d, handle 0x%x\n";

uint16_t file_read(uint16_t handle, void* dst, uint16_t size) {
  FIL* file = (FIL*) handle;
  int index = get_handle_index(file);
  if (index < 0 || !file_handle_active[index])
    return 0;

  UINT size_read = 0;
  int status = f_read(file, dst, size, &size_read);
#ifdef DEBUG
  if (status != FR_OK) {
    fprintf_P(stderr, file_read_str0, size_read, status, handle);
  }
#endif
  return size_read;
}

const char file_write_str0[] PROGMEM =
    "f_write() of %u bytes returned status %d\n";

uint16_t file_write(uint16_t handle, const void* src, uint16_t size) {
  FIL* file = (FIL*) handle;
  int index = get_handle_index(file);
  if (index < 0 || file_handle_active[index])
    return 0;

  UINT size_written;
  int status = f_write(file, src, size, &size_written);
#ifdef DEBUG
  if (status != FR_OK) {
    fprintf_P(stderr, file_write_str0, size_written, status);
  }
#endif

  return size_written;
}

uint32_t file_size(uint16_t handle) {
  return f_size((FIL*) handle);
}

void file_seek(uint16_t handle, uint32_t offset) {
  f_lseek((FIL*) handle, offset);
}
