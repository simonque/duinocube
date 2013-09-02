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
  const char* filename = "foo.txt";

  // Open the file.
  // TODO: add mode definitions.
  uint16_t handle = DC.rpcFileOpen(filename, 0x01);
  uint16_t size_read;
  if (!handle) {
    Serial.println("Could not open file.");
  } else {
    Serial.print("Opened file handle 0x");
    Serial.println(handle, HEX);
 
    size_read = DC.rpcFileRead(handle, addr, sizeof(buf));
    Serial.print("Read ");
    Serial.print(size_read);
    Serial.println(" bytes from file.");

    // Copy the file contents into a buffer.
    memset(buf, 0, sizeof(buf));
    DC.readSharedRAM(addr, buf, size_read);
    Serial.print("Read string: ");
    Serial.println(buf);

    DC.rpcFileClose(handle);
  }

  // TODO: Test other file operations.

  Serial.println("End of test.");
  while(1);
}

