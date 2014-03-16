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

// DuinoCube system shield memory allocation test.

#include <DuinoCube.h>
#include <SPI.h>

static void test_alloc();
static void test_access();

void setup() {
  Serial.begin(115200);

  DC.begin();
}

void loop() {
  test_alloc();
  test_access();

  printf("End of test.\n");
  while(1);
}

#define EXPECT_EQ(value, expected) \
    expect_equal(value, #value, expected, #expected, __func__, __LINE__)
#define EXPECT_NE(value, unexpected) \
    expect_unequal(value, #value, unexpected, #unexpected, __func__, __LINE__)

void expect_equal(uint16_t value, const char* value_string,
                  uint16_t expected, const char* expected_string,
                  const char* func, int line) {
  if (value == expected)
    return;
  printf("%s %d: Value mismatch:  Expected |%s| = %u, actual |%s| = %u\n",
         func, line, expected_string, expected, value_string, value);
}

void expect_unequal(uint16_t value, const char* value_string,
                    uint16_t unexpected, const char* unexpected_string,
                    const char* func, int line) {
  if (value != unexpected)
    return;
  printf("%s %d: Value |%s| should not match unexpected value: %s = %u\n",
         func, line, value_string, unexpected_string, unexpected);
}

// Defines what to do for an iteration of the alloc test.
#define COMMAND_ALLOC   0
#define COMMAND_FREE    1

struct TestCommand {
  uint8_t command;          // Command code: Alloc vs free.
  uint16_t size;            // Size to alloc, if alloc'ing. 
  uint16_t addr_index;      // For alloc: store addr in slot with this index.
                            // For free: free addr in slot with this index.
  int16_t delta_total;      // Change in total free memory after this command.
  int16_t delta_largest;    // Change in largest free region after this command.
};

#define BLOCK_SIZE          SHARED_MEMORY_BLOCK_SIZE
#define MAX_ALLOC_ADDRS     8

static uint16_t alloc_addrs[MAX_ALLOC_ADDRS];

static const TestCommand kTestCommands[] = {
  // Zero size allocation should change nothing.
  { COMMAND_ALLOC, 0, 0, 0, 0 },
  // Even a small allocation should reserve a whole block.
  { COMMAND_ALLOC, 1, 0, -BLOCK_SIZE, -BLOCK_SIZE },
  // Allocate one unit less than the block size.  Should still allocate an
  // entire block.
  { COMMAND_ALLOC, BLOCK_SIZE - 1, 1, -BLOCK_SIZE, -BLOCK_SIZE },
  // Allocate exactly one block.
  { COMMAND_ALLOC, BLOCK_SIZE, 2, -BLOCK_SIZE, -BLOCK_SIZE },
  // Allocate one block and then some.
  { COMMAND_ALLOC, BLOCK_SIZE + 5, 3, -BLOCK_SIZE * 2, -BLOCK_SIZE * 2 },
  // Allocate three blocks.
  { COMMAND_ALLOC, BLOCK_SIZE * 3, 4, -BLOCK_SIZE * 3, -BLOCK_SIZE * 3 },
  // Allocate three blocks and then some.
  { COMMAND_ALLOC, BLOCK_SIZE * 3 + 5, 5, -BLOCK_SIZE * 4, -BLOCK_SIZE * 4 },
  // Attempt to allocate a little more than the remaining space.  So far, 12 blocks
  // have been allocated.  Should fail.
  { COMMAND_ALLOC, SHARED_MEMORY_HEAP_SIZE - BLOCK_SIZE * 12 + 1, 6, 0, 0 },
  // Attempt to allocate all the remaining space.
  { COMMAND_ALLOC, SHARED_MEMORY_HEAP_SIZE - BLOCK_SIZE * 12, 6,
    -(SHARED_MEMORY_HEAP_SIZE - BLOCK_SIZE * 12),
    -(SHARED_MEMORY_HEAP_SIZE - BLOCK_SIZE * 12) },
  // Free a block, should increase the free size.
  { COMMAND_FREE, 0, 5, BLOCK_SIZE * 4, BLOCK_SIZE * 4 },
  // Free smaller non-contiguous blocks, should increase total free size but not
  // the largest free size.
  { COMMAND_FREE, 0, 3, BLOCK_SIZE * 2, 0 },
  { COMMAND_FREE, 0, 1, BLOCK_SIZE, 0 },
  { COMMAND_FREE, 0, 0, BLOCK_SIZE, 0 },

  // Free the region allocated in slot 4.
  // The largest size should be increased by the sizes of addresses in slots 3
  // and 4 as they are joined with the largest existing freed region.
  { COMMAND_FREE, 0, 4, BLOCK_SIZE * 3, BLOCK_SIZE * (3 + 2) },

  // Now allocate and free several blocks.  This should be in the middle of the
  // largest freed region.
  { COMMAND_ALLOC, BLOCK_SIZE * 7, 4, -BLOCK_SIZE * 7, -BLOCK_SIZE * 7 },
  { COMMAND_FREE, 0, 4, BLOCK_SIZE * 7, BLOCK_SIZE * 7 },

  // Free the allocated region in slot 2.
  // The largest size should be increased by the sizes of addresses in slots 0,
  // 1, and 2 as they are joined with the largest existing freed region.
  { COMMAND_FREE, 0, 2, BLOCK_SIZE, BLOCK_SIZE * (1 + 1 + 1) },

  // Free the allocated region in slot 6.
  { COMMAND_FREE, 0, 6, (SHARED_MEMORY_HEAP_SIZE - BLOCK_SIZE * 12),
    (SHARED_MEMORY_HEAP_SIZE - BLOCK_SIZE * 12) },
};

// Tests allocation/freeing of shared memory.
static void test_alloc() {
  // Get initial memory stats.
  uint16_t prev_total, prev_largest;
  DC.Mem.stat(&prev_total, &prev_largest);

  printf("Total free memory: %d\n", prev_total);
  printf("Largest block of free memory: %d\n", prev_largest);

  // Clear allocated address array.
  memset(alloc_addrs, 0, sizeof(alloc_addrs));

  for (int i = 0; i < sizeof(kTestCommands) / sizeof(kTestCommands[0]); ++i) {
    const TestCommand& command = kTestCommands[i];
    uint16_t addr_index = command.addr_index;
    printf("Processing command #%d: code = %u\n", i, command.command);
    switch (command.command) {
    case COMMAND_ALLOC: {
      uint16_t addr = DC.Mem.alloc(command.size);
      // If the allocation is expected to be unsuccessful, make sure a NULL
      // address is returned.
      if (command.delta_total < 0)
        EXPECT_NE(addr, 0);
      else
        EXPECT_EQ(addr, 0);
      if (addr && addr_index < MAX_ALLOC_ADDRS)
        alloc_addrs[addr_index] = addr;
      break;
    }
    case COMMAND_FREE:
      if (addr_index < MAX_ALLOC_ADDRS && alloc_addrs[addr_index]) {
        DC.Mem.free(alloc_addrs[addr_index]);
        alloc_addrs[addr_index] = 0;
      }
      break;
    default:
      printf("Invalid command code: %u\n", command.command);
      continue;
    }

    // Get new stats and make sure they are what's expected.
    uint16_t new_total, new_largest;
    DC.Mem.stat(&new_total, &new_largest);
    EXPECT_EQ(new_total, prev_total + command.delta_total);
    EXPECT_EQ(new_largest, prev_largest + command.delta_largest);

    prev_total = new_total;
    prev_largest = new_largest;
  }
}

// Tests read/write of shared memory.
static void test_access() {
  const char string[] = "The quick brown fox jumps over the lazy dog.";
  const int kBufferSize = 1024;
  char buffer[kBufferSize];

  // Allocate a shared memory block and clear it.
  uint16_t addr = DC.Mem.alloc(sizeof(buffer));
  EXPECT_NE(addr, 0);
  memset(buffer, 0, sizeof(buffer));
  DC.Mem.write(addr, buffer, sizeof(buffer));

  // Read it back to make sure it was cleared.
  DC.Mem.read(addr, buffer, sizeof(buffer));
  for (uint16_t i = 0; i < sizeof(buffer) / sizeof(buffer[0]); ++i)
    EXPECT_EQ(buffer[i], 0);

  // Write test string to RAM and read it back.
  DC.Mem.write(addr, string, strlen(string) + 1);
  memset(buffer, 0, sizeof(buffer));
  DC.Mem.read(addr, buffer, strlen(string) + 1);

  printf("Read back: %s\n", buffer);
  printf("Expecting: %s\n", string);

  DC.Mem.free(addr);
}
