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

// DuinoCube system shield RPC test.

#include <DuinoCube.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);

  DC.begin();
  DC.resetRPCServer();
}

void loop() {
  uint16_t addr = 0x100;
  char buf[256];

  // Call Hello(), should get a "hello world" string back.
  DC.rpcHello(addr);
  DC.readSharedRAM(addr, buf, sizeof(buf));
  Serial.print("Hello() returned: ");
  Serial.println(buf);

  // Call Invert() to flip all the bits in a string.
  // Calling it twice will return the same string.
  const char str[] = "The quick brown fox jumps over the lazy dog.";
  DC.writeSharedRAM(addr, str, strlen(str));
  DC.rpcInvert(addr, strlen(str));
  DC.readSharedRAM(addr, buf, strlen(str));
  Serial.print("Invert(str) = ");
  Serial.println(buf);

  DC.rpcInvert(addr, strlen(str));
  DC.readSharedRAM(addr, buf, strlen(str));
  Serial.print("Invert(Invert(str)) = ");
  Serial.println(buf);

  while(1);
}

