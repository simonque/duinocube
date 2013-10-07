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

// DuinoCube shared memory functions.

#include <string.h>

#include "DuinoCube_mem.h"

#include "defines.h"
#include "printf.h"
#include "spi.h"

#include "shmem.h"

#define RAM_WRITE    2
#define RAM_READ     3

// Record of a block on the shared memory heap.
struct HeapBlock {
  // If |num_blocks_in_region| == 0, it means the is not the first block in a
  // contiguous allocated or free region.  Only the first block in a same-status
  // region will have this field set to the total number of blocks in that
  // region.
  uint8_t num_blocks_in_region:7;
  uint8_t is_allocated:1;   // 0 = Free block, 1 = Allocated block.
};

#define NUM_HEAP_BLOCKS (SHARED_MEMORY_HEAP_SIZE / SHARED_MEMORY_BLOCK_SIZE)
static HeapBlock heap_blocks[NUM_HEAP_BLOCKS];

const char shmem_init_str0[] PROGMEM =
    "Shared memory heap initialized with %u bytes: \n";

void shmem_init() {
  // Initialize the heap.
  memset(heap_blocks, 0, sizeof(heap_blocks));
  heap_blocks[0].num_blocks_in_region = NUM_HEAP_BLOCKS;
#ifdef DEBUG
  printf_P(shmem_init_str0, SHARED_MEMORY_HEAP_SIZE);
#endif
}

void shmem_read(uint16_t addr, void* data, uint16_t len) {
  char* buf = (char*)data;
  spi_set_ss(DEV_SELECT_LOGIC);
  spi_tx(OP_ACCESS_RAM);

  spi_tx(RAM_READ);
  spi_tx(addr >> 8);
  spi_tx((uint8_t) addr);
  for (uint16_t i = 0; i < len; ++i)
    buf[i] = spi_tx(0);

  spi_set_ss(DEV_SELECT_NONE);
}

void shmem_write(uint16_t addr, const void* data, uint16_t len) {
  const char* buf = (const char*)data;

  if (addr < SHARED_MEMORY_SIZE) {
    // Writing to generic shared memory.
    spi_set_ss(DEV_SELECT_LOGIC);
    spi_tx(OP_ACCESS_RAM);

    spi_tx(RAM_WRITE);
    spi_tx(addr >> 8);
    spi_tx((uint8_t) addr);
    for (uint16_t i = 0; i < len; ++i)
      spi_tx(buf[i]);
  } else {
    // Writing to core memory space.
    // TODO: get rid of magic number.
    spi_set_ss(DEV_SELECT_FPGA);
    addr = (addr - SHARED_MEMORY_SIZE) | 0x8000;

    spi_tx(addr >> 8);
    spi_tx((uint8_t) addr);
    for (uint16_t i = 0; i < len; ++i)
      spi_tx(buf[i]);
  }
  spi_set_ss(DEV_SELECT_NONE);
}

void shmem_stat(uint16_t* total_free_size, uint16_t* largest_free_size) {
  uint16_t num_free_blocks = 0;
  uint16_t largest_free_region_num_blocks = 0;
  uint16_t current_free_region_num_blocks = 0;
  for (uint16_t i = 0;
       i < NUM_HEAP_BLOCKS && heap_blocks[i].num_blocks_in_region > 0;
       i += heap_blocks[i].num_blocks_in_region) {
    const HeapBlock& block = heap_blocks[i];
    if (!block.is_allocated) {
      // Count the number of free blocks.
      num_free_blocks += block.num_blocks_in_region;
      current_free_region_num_blocks += block.num_blocks_in_region;
      // Save the current region size if it is the largest so far.
      if (current_free_region_num_blocks > largest_free_region_num_blocks)
        largest_free_region_num_blocks = current_free_region_num_blocks;
      continue;
    }
    // When the iteration encounters an allocated block, reset the current
    // region counter.
    current_free_region_num_blocks = 0;
  }

  if (total_free_size)
    *total_free_size = num_free_blocks * SHARED_MEMORY_BLOCK_SIZE;
  if (largest_free_size) {
    *largest_free_size =
        largest_free_region_num_blocks * SHARED_MEMORY_BLOCK_SIZE;
  }
}

uint16_t shmem_alloc(uint16_t size) {
  if (size == 0)
    return (uint16_t) NULL;
  uint16_t i;
  uint16_t num_blocks_to_alloc =
      (size + SHARED_MEMORY_BLOCK_SIZE - 1)/ SHARED_MEMORY_BLOCK_SIZE;
  for (i = 0;
       i < NUM_HEAP_BLOCKS && heap_blocks[i].num_blocks_in_region > 0;
       i += heap_blocks[i].num_blocks_in_region) {
    const HeapBlock& block = heap_blocks[i];
    // Skip blocks that are not free or are too small.
    // This will return the first block that fits.  It may not be the smallest
    // block that fits.
    if (!block.is_allocated &&
        block.num_blocks_in_region >= num_blocks_to_alloc) {
      break;
    }
  }

  // Return NULL if no free region was found.  The heap should not start at 0.
  if (i == NUM_HEAP_BLOCKS)
    return (uint16_t) NULL;
  // When a block has been found, mark it as allocated, along with subsequent
  // blocks in the allocated region.
  if (heap_blocks[i].num_blocks_in_region > num_blocks_to_alloc) {
    // Mark extra unallocated blocks at the end if there are any.
    heap_blocks[i + num_blocks_to_alloc].num_blocks_in_region =
        heap_blocks[i].num_blocks_in_region - num_blocks_to_alloc;
  }
  heap_blocks[i].num_blocks_in_region = num_blocks_to_alloc;
  for (uint16_t j = i; j < i + num_blocks_to_alloc; ++j)
    heap_blocks[j].is_allocated = true;
  return i * SHARED_MEMORY_BLOCK_SIZE + SHARED_MEMORY_HEAP_START;
}

const char shmem_free_str0[] PROGMEM = "%s: Invalid address: 0x%04x\n";
const char shmem_free_str1[] PROGMEM =
    "%s: Not an allocated head block: 0x%04x\n";

void shmem_free(uint16_t addr) {
  // Do not attempt to free a non-aligned address.
  // Do not attempt to free a non-heap address.
  if (addr % SHARED_MEMORY_BLOCK_SIZE != 0 ||
      addr < SHARED_MEMORY_HEAP_START) {
#ifdef DEBUG
    printf_P(shmem_free_str0, __func__, addr);
#endif
    return;
  }
  uint16_t block_index =
      (addr - SHARED_MEMORY_HEAP_START)/ SHARED_MEMORY_BLOCK_SIZE;
  // Make sure the given address actually points to the start of an allocated
  // region in the heap.
  if (!heap_blocks[block_index].is_allocated) {
#ifdef DEBUG
    printf_P(shmem_free_str1, __func__, addr);
#endif
    return;
  }
  uint16_t num_blocks = heap_blocks[block_index].num_blocks_in_region;
  // Clear the allocated flags.
  memset(heap_blocks + block_index, 0, num_blocks * sizeof(*heap_blocks));
  heap_blocks[block_index].num_blocks_in_region = num_blocks;

  // If the next block is also free, merge the two blocks.
  if (block_index + num_blocks < NUM_HEAP_BLOCKS &&
      !heap_blocks[block_index + num_blocks].is_allocated) {
    heap_blocks[block_index].num_blocks_in_region +=
        heap_blocks[block_index + num_blocks].num_blocks_in_region;
    heap_blocks[block_index + num_blocks].num_blocks_in_region = 0;
  }

  // If the previous block is also free, merge the two blocks.
  if (block_index == 0)   // This means there's no previous block.
    return;
  // This means the previous block is not free.
  if (heap_blocks[block_index - 1].is_allocated)
    return;
  for (uint16_t i = block_index - 1; true; --i) {
    if (heap_blocks[i].num_blocks_in_region == 0) {
      if (i > 0)
        continue;
      break;
    }
    // There must be a previous block whose head is heap_blocks[i] if this code
    // is reached.
    heap_blocks[i].num_blocks_in_region +=
        heap_blocks[block_index].num_blocks_in_region;
    heap_blocks[block_index].num_blocks_in_region = 0;
    break;
  }
}
