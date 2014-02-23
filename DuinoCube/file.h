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

// DuinoCube File I/O library for Arduino.

#ifndef __DUINOCUBE_FILE_H__
#define __DUINOCUBE_FILE_H__

#include <stdint.h>

// Taken from FatFS.
// TODO: Add more defines.
#define FILE_READ_ONLY    0x01

namespace DuinoCube {

class File {
 public:
  // File I/O functions.
  static uint16_t open(const char* filename, uint16_t mode);
  static void close(uint16_t handle);
  static uint16_t read(uint16_t handle, uint16_t dst_addr, uint16_t size);
  static uint16_t write(uint16_t handle, uint16_t src_addr, uint16_t size);
  static uint16_t readToCore(uint16_t handle, uint16_t dst_addr, uint16_t size);

  // Returns the size of the file in bytes.
  static uint32_t size(uint16_t handle);
  // Move the file access pointer to |offset| bytes from the start.  It can also
  // increase the file size.
  static void seek(uint16_t handle, uint32_t offset);
};

}  // namespace DuinoCube

#endif  // __DUINOCUBE_FILE_H__
