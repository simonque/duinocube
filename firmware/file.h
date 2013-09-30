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

#ifndef __FILE_H__
#define __FILE_H__

#include <stdint.h>

// Initializes file system.
void file_init();

// Open a file using a free file handle, if one exists.  Returns pointer to
// handle, or NULL if it failed.
uint16_t file_open(const char* filename, uint16_t mode);

// Close a file handle indicated by |handle|.
void file_close(uint16_t handle);

// Basic file I/O.  Returns the number of bytes read/written.
uint16_t file_read(uint16_t handle, void* dst, uint16_t size);
uint16_t file_write(uint16_t handle, const void* src, uint16_t size);

// Gets size of file.
uint32_t file_size(uint16_t handle);

// Move file access pointer.
void file_seek(uint16_t handle, uint32_t offset);

#endif  // __FILE_H__
