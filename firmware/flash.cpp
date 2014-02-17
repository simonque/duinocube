// Copyright (C) 2014 Simon Que
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

// DuinoCube FPGA flash reprogram functions.

#include "flash.h"

#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "file.h"
#include "spi.h"

// Control pin definitions.
#define FLASH_DDR                   DDRC
#define FLASH_PORT                  PORTC
#define FLASH_SELECT_BIT            PORTC4

// S25FL216K size defs.
#define SECTOR_SIZE             0x1000    // 4 kB sectors.
#define BLOCK_SIZE             0x10000    // 64 kB blocks.
#define CHIP_SIZE             0x200000    // 2 MB chip.
#define PAGE_SIZE                0x100    // 256 bytes in a writable page.

// Status register bit definitions.
#define STATUS_REG_WIP               0    // Write in progress.
#define STATUS_REG_WEL               1    // Write enable latch.

// Command codes.
#define COMMAND_WRITE_ENABLE      0x06
#define COMMAND_READ_STATUS       0x05
#define COMMAND_READ_DATA         0x03
#define COMMAND_PAGE_PROGRAM      0x02
#define COMMAND_BLOCK_ERASE       0xd8
#define COMMAND_CHIP_ERASE        0xc7
#define COMMAND_JEDEC_ID          0x9f

// Expected JEDEC ID of S25FL216K.
#define EXPECTED_DEVICE_ID    0x014015

// Erased memory has this value. This is used to determine parts of the data
// that are empty and don't need to be written.
#define UNINITIALIZED_MEMORY_VALUE  0xff

// Returns an XOR checksum of the provided data.
uint32_t get_checksum(const char* data, uint32_t size) {
  const uint32_t* data32 = reinterpret_cast<const uint32_t*>(data);
  uint32_t checksum = 0;

  // The data size may not be a multiple of uint32_t. Read the remainder
  // separately so the extra bytes can be zeroed.
  uint8_t remainder = size % sizeof(checksum);

  while (size > remainder) {
    checksum ^= *data32++;
    size -= sizeof(checksum);
  }

  uint32_t remaining_data = *data32;
  char* remaining_bytes = reinterpret_cast<char*>(&remaining_data);
  for (uint8_t i = 0; i < remainder; ++i) {
    remaining_bytes[sizeof(remaining_data) - 1 - i] = 0;
  }
  checksum ^= remaining_data;

  return checksum;
}

// Assert / de-assert flash select pin.
static void flash_spi_select() {
  FLASH_PORT &= ~(1 << FLASH_SELECT_BIT);
}
static void flash_spi_deselect() {
  FLASH_PORT |= (1 << FLASH_SELECT_BIT);
}

// Starts a command operation.
static void start_command(uint8_t command) {
  flash_spi_select();
  spi_tx(command);
}

// Ends a command operation.
static void end_command() {
  flash_spi_deselect();
}

// Sends a 24-bit address over the SPI.
static void write_address(uint32_t address) {
  const char* address_bytes = reinterpret_cast<const char*>(&address);
  spi_tx(address_bytes[2]);
  spi_tx(address_bytes[1]);
  spi_tx(address_bytes[0]);
}

// Write enable.
static void write_enable() {
  start_command(COMMAND_WRITE_ENABLE);
  end_command();
}

// Read status reg.
static uint8_t read_status_reg() {
  start_command(COMMAND_READ_STATUS);
  uint8_t value = spi_tx(0);
  end_command();

  return value;
}

// Read data.
static void read_data(uint32_t offset, char* buf, uint32_t size) {
  start_command(COMMAND_READ_DATA);
  write_address(offset);

  // The file data bit order is reversed.
  spi_set_bit_order(SPI_LSB_FIRST);
  for (; size > 0; --size, ++buf) {
    *buf = spi_tx(0);
  }
  spi_set_bit_order(SPI_MSB_FIRST);

  end_command();
}

// Programs a page of data.
static void page_program(uint32_t offset, const char* buf, uint16_t size) {
  start_command(COMMAND_PAGE_PROGRAM);
  write_address(offset);
  if (size > PAGE_SIZE) {
    size = PAGE_SIZE;
  }

  // The file data bit order is reversed.
  spi_set_bit_order(SPI_LSB_FIRST);
  for (; size > 0; --size, ++buf) {
    spi_tx(*buf);
  }
  spi_set_bit_order(SPI_MSB_FIRST);

  end_command();
}

// Initiates block erase.
static void block_erase(uint32_t offset) {
  start_command(COMMAND_BLOCK_ERASE);
  write_address(offset);
  end_command();
}

// Initiates chip erase.
static void chip_erase() {
  start_command(COMMAND_CHIP_ERASE);
  end_command();
}

// Reads the JEDEC ID.
static uint32_t read_device_id() {
  start_command(COMMAND_JEDEC_ID);
  uint32_t id = 0;
  char* id_bytes = reinterpret_cast<char*>(&id);
  id_bytes[2] = spi_tx(0);
  id_bytes[1] = spi_tx(0);
  id_bytes[0] = spi_tx(0);
  end_command();

  return id;
}

void flash_init() {
  // Deselect the flash interface before setting the pin as an output.
  flash_spi_deselect();
  FLASH_DDR |= (1 << FLASH_SELECT_BIT);
}

uint16_t flash_reprogram(uint16_t handle) {
  // Determine file size.
  uint32_t data_size = file_size(handle);

  // Check device ID.
  uint32_t id = read_device_id();
  printf("ID: 0x%08lx\n", id);
  if (id != EXPECTED_DEVICE_ID) {
    return FLASH_PROGRAM_ID_MISMATCH;
  }

  // Erase enough sectors to fit the new configuration file.
#if defined(DEBUG)
  printf_P(PSTR("Erasing chip\n"));
#endif  // defined (DEBUG)
  // Set write enable.
  write_enable();
  // Start block erase.
  chip_erase();
  // Make sure WIP bit is set.
  if (!(read_status_reg() & (1 << STATUS_REG_WIP))) {
    return FLASH_PROGRAM_ERASE_START_ERROR;
  }
  // Wait for WIP = 0.
  // TODO: add timeout.
  while (read_status_reg() & (1 << STATUS_REG_WIP));

  // Write the new data to flash.
  char file_buf[PAGE_SIZE];
  for (uint32_t offset = 0; offset < data_size; offset += PAGE_SIZE) {
    // Read the file.
    uint16_t size_read = file_read(handle, file_buf, PAGE_SIZE);
    if (size_read < PAGE_SIZE && size_read != offset - data_size) {
      return FLASH_PROGRAM_FILE_READ_ERROR;
    }

    bool page_empty = true;
    for (uint16_t page_offset = 0; page_offset < size_read; ++page_offset) {
      if (file_buf[page_offset] != UNINITIALIZED_MEMORY_VALUE) {
        page_empty = false;
        break;
      }
    }

    if (page_empty) {
#if defined(DEBUG)
      printf_P(PSTR("No data at 0x%08lx\n"), offset);
#endif  // defined(DEBUG)
      continue;
    }

    // Obtain a 32-bit checksum.
    uint32_t checksum = get_checksum(file_buf, size_read);

    // Program a |PAGE_SIZE| page of the flash.
    write_enable();
#if defined(DEBUG)
    printf_P(PSTR("Programming data at 0x%08lx\n"), offset);
#endif  // defined(DEBUG)
    page_program(offset, file_buf, size_read);

    // Make sure WIP bit is set.
    if (!(read_status_reg() & (1 << STATUS_REG_WIP))) {
      return FLASH_PROGRAM_WRITE_START_ERROR;
    }
    // Wait for programming to finish.
    // TODO: add timeout.
    while (read_status_reg() & (1 << STATUS_REG_WIP));

    // Read back and check the checksum.
    read_data(offset, file_buf, size_read);
    uint32_t new_checksum = get_checksum(file_buf, size_read);

    if (checksum != new_checksum) {
      return FLASH_PROGRAM_WRITE_VERIFY_ERROR;
    }
  }

  return FLASH_PROGRAM_SUCCESS;
}
