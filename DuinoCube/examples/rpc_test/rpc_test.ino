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

// DuinoCube system shield RPC test.

#include <DuinoCube.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);

  DC.begin();
}

void loop() {
  uint16_t addr = 0x100;
  char buf[256];

  // Call Hello(), should get a "hello world" string back.
  DC.RPC.hello(addr);
  DC.Mem.read(addr, buf, sizeof(buf));
  printf("Hello() returned: %s\n", buf);

  // Call Invert() to flip all the bits in a string.
  // Calling it twice will return the same string.
  const char str[] = "The quick brown fox jumps over the lazy dog.";
  int string_length = strlen(str);
  buf[string_length] = 0;   // Add a null terminator to the destination buffer.
  DC.Mem.write(addr, str, string_length);
  DC.RPC.invert(addr, string_length);
  DC.Mem.read(addr, buf, string_length);
  printf("Invert(str) = %s\n", buf);

  DC.RPC.invert(addr, string_length);
  DC.Mem.read(addr, buf, string_length);
  printf("Invert(Invert(str)) = %s\n", buf);

  while(1);
}

