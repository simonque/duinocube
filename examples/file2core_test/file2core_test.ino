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

// Test the coprocessor's direct copying of data from a file to Core shield.

#include <DuinoCube.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);

  DC.begin();

  // Run test only once.
  run_once();
}

void loop() {
}

static void run_once() {
  const char kData16kFile[] = "data16k";
  test_file_copy(kData16kFile, SPRITE_BASE, 0, NUM_SPRITES * NUM_SPRITE_REGS * sizeof(uint16_t));
  test_file_copy(kData16kFile, TILEMAP_BASE, TILEMAP_BANK, TILEMAP_SIZE * NUM_TILEMAPS);
  for (uint16_t bank = VRAM_BANK_BEGIN; bank < VRAM_BANK_END; ++bank)
    test_file_copy(kData16kFile, VRAM_BASE, bank, VRAM_BANK_SIZE);

  Serial.println("End of test.");
}

static void test_file_copy(const char* filename,
                           uint16_t addr,
                           uint16_t bank,
                           uint16_t size) {
  // TODO: add mode definitions.
  uint16_t handle = DC.File.open(filename, 0x01);

  if (!handle) {
    Serial.println("Could not open file.");
    return;
  }
  Serial.println("Opened file.");

  Serial.print("Size in bytes: 0x");
  Serial.println(size, HEX);

  if (size != (uint16_t) size) {
    Serial.println("Size is too large, must fit in uint16_t.");
    return;
  }

  uint32_t file_size = DC.File.size(handle);
  if (file_size < size) {
    Serial.print("File size is too small: ");
    Serial.println(file_size, HEX);
  }

  // Allocate some shared memory.
  uint16_t buf_addr = DC.Mem.alloc(size);
  if (!buf_addr) {
    Serial.println("Unable to allocate shared memory.");
    return;
  }

  uint16_t size_read = DC.File.read(handle, buf_addr, size);
  if (size_read != size) {
    Serial.print("Read 0x");
    Serial.print(size_read, HEX);
    Serial.println(" bytes, did not match expected size.");
    return;
  }

  // Compute a checksum of the file data by adding it all up.
  uint64_t checksum = 0;
  for (uint16_t offset = 0; offset < size; offset += sizeof(checksum)) {
    uint64_t value;
    DC.Sys.readSharedRAM(buf_addr + offset, &value, sizeof(value));
    checksum += value;
  }
  DC.Mem.free(buf_addr);

  Serial.print("Data checksum is 0x");
  char checksum_string[32];
  sprintf(checksum_string, "%08lx", (checksum >> 32));
  sprintf(checksum_string + strlen(checksum_string), "%08lx",
          (uint32_t) checksum);
  Serial.println(checksum_string);

  // Reset file pointer.
  DC.File.seek(handle, 0);

  // Now copy the file directly to Core video memory.
  DC.Core.writeWord(REG_MEM_BANK, bank);
  DC.Core.writeWord(REG_SYS_CTRL, 1);
  size_read = DC.File.readToCore(handle, addr, size);
  if (size_read != size) {
    Serial.print("Read 0x");
    Serial.print(size_read, HEX);
    Serial.println(" bytes, did not match expected size.");
    return;
  }

  DC.File.close(handle);

  // Read the data back from Core memory.
  uint64_t core_checksum = 0;
  for (uint16_t offset = 0; offset < size; offset += sizeof(core_checksum)) {
    uint64_t value;
    DC.Core.readData(addr + offset, &value, sizeof(value));
    core_checksum += value;
  }
  DC.Core.writeWord(REG_SYS_CTRL, 0);
  DC.Core.writeWord(REG_MEM_BANK, 0);

  Serial.print("Core checksum is 0x");
  sprintf(checksum_string, "%08lx", (core_checksum >> 32));
  sprintf(checksum_string + strlen(checksum_string), "%08lx",
          (uint32_t) core_checksum);
  Serial.println(checksum_string);

  if (core_checksum != checksum) {
    Serial.println("File and core data checksums do not match!");
    return;
  }
}

