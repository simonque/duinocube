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

// Initializes file system.
void file_init();

// Open a file using a free file handle, if one exists.  Returns handle number
// in |handle|.
int file_open(const char* filename, uint8_t mode, uint16_t* handle);

// Close a file handle indicated by |handle|.
int file_close(uint16_t handle);

// Basic file I/O.
int file_read(uint16_t handle, void* dst, uint16_t size);
int file_write(uint16_t handle, const void* src, uint16_t size);

#endif  // __FILE_H__
