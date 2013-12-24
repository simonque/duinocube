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

// DuinoCube system shield file I/O test.

#include <DuinoCube.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  DC.begin();
}

void loop() {
  uint16_t addr = 0x100;
  char buf[256];
  const char* filename = "foo.txt";

  // Open the file.
  // TODO: add mode definitions.
  uint16_t handle = DC.File.open(filename, 0x01);
  uint16_t size_read;
  if (!handle) {
    Serial.println("Could not open file.");
  } else {
    Serial.print("Opened file handle 0x");
    Serial.println(handle, HEX);
 
    size_read = DC.File.read(handle, addr, sizeof(buf));
    Serial.print("Read ");
    Serial.print(size_read);
    Serial.println(" bytes from file.");

    uint32_t expected_size = DC.File.size(handle);
    if (expected_size == size_read) {
      Serial.println("Size read matches expected size.");
    } else {
      Serial.print("Size mismatch, expected: ");
      Serial.print(expected_size);
      Serial.println(" bytes.");
    }

    // Copy the file contents into a buffer.
    memset(buf, 0, sizeof(buf));
    DC.Sys.readSharedRAM(addr, buf, size_read);
    Serial.print("Read string: ");
    Serial.println(buf);

    // Clear the shared memory.
    memset(buf, 0, sizeof(buf));
    DC.Sys.writeSharedRAM(addr, buf, sizeof(buf));

    // Reset the file pointer halfway and do it again.
    DC.File.seek(handle, size_read / 2);
    DC.File.read(handle, addr, sizeof(buf));
    DC.Sys.readSharedRAM(addr, buf, size_read);
    Serial.print("Half string: ");
    Serial.println(buf);

    DC.File.close(handle);
  }

  // TODO: Test other file operations.

  Serial.println("End of test.");
  while(1);
}

