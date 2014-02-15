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

// AVR reprogram functions.

#include "isp.h"

#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "file.h"
#include "spi.h"
#include "timer.h"
#include "utils.h"

// Control pin definitions.
#define ISP_DDR                   DDRB
#define ISP_PORT                  PORTB
#define ISP_RESET_BIT             PORTB2

#define PROGRAM_MEMORY_PAGE_SIZE  0x80    // 128 bytes per page.

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

// Expected signature bytes of ATmega328P.
#define EXPECTED_DEVICE_ID    0x1e950f

// Erased memory has this value. This is used to determine parts of the data
// that are empty and don't need to be written.
#define UNINITIALIZED_MEMORY_VALUE  0xff

// Returns an XOR checksum of the provided data.
static uint32_t get_checksum(const void* data, uint32_t size) {
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

// Assert / de-assert ISP reset pin.
static void isp_reset() {
  ISP_PORT &= ~(1 << ISP_RESET_BIT);
}
static void isp_release() {
  ISP_PORT |= (1 << ISP_RESET_BIT);
}
/*
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
  if (size > page_size) {
    size = page_size;
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
*/

// Programming enable sequence.
const uint8_t kProgEnableCommand[] = { 0xac, 0x53, 0x00, 0x00 };

static bool isp_enable() {
  // The SPI clock period must be < f_target. In this case, f_target = 16 MHz.
  // f_SPI = 16 MHz / 6 = 2.67 MHz. A frequency of 2.5 MHz or f_clk / 8 is
  // sufficient.
  SPCR |= (0 << SPR1) | (1 << SPR0);

  // SCK must be low at this point. This function assumes that the SPI interface
  // has been properly initialized.
  isp_reset();

  // Wait 20 ms.
  timer_reset();
  while (timer_get_ms() < 20);

  uint8_t result[ARRAY_SIZE(kProgEnableCommand)];
  for (uint8_t i = 0; i < ARRAY_SIZE(kProgEnableCommand); ++i) {
    result[i] = spi_tx(kProgEnableCommand[i]);
  }
  uint8_t echo_value = result[2];

  printf("echo: 0x%x\n", echo_value);
  return (echo_value == 0x53);
}

// Read device signature.
const uint8_t kReadDeviceIDCommand[] = { 0x30, 0x00 };
static uint32_t read_device_id() {
  uint32_t id = 0;
  char* id_bytes = reinterpret_cast<char*>(&id);
  for (uint8_t byte_index = 0; byte_index < 3; ++byte_index) {
    spi_tx(kReadDeviceIDCommand[0]);
    spi_tx(kReadDeviceIDCommand[1]);
    spi_tx(byte_index);
    id_bytes[2 - byte_index] = spi_tx(0);
  }
  return id;
}

// Initiate program memory / EEPROM erase.
const uint8_t kChipEraseCommand[] = { 0xac, 0x80, 0x00, 0x00 };
static void chip_erase() {
  for (uint8_t i = 0; i < ARRAY_SIZE(kChipEraseCommand); ++i) {
    spi_tx(kChipEraseCommand[i]);
  }
}

// Poll for ready status.
const uint8_t kPollReadyCommand[] = { 0xf0, 0x00, 0x00 };
static uint8_t poll_ready() {
  for (uint8_t i = 0; i < ARRAY_SIZE(kPollReadyCommand); ++i) {
    spi_tx(kPollReadyCommand[i]);
  }
  return spi_tx(0);
}

// Read program memory.
#define READ_PROGRAM_MEMORY_LO      0x28
#define READ_PROGRAM_MEMORY_HI      0x20
static uint8_t read_program_memory_byte(uint8_t command, uint16_t addr) {
  spi_tx(command);
  spi_tx((uint8_t)(addr >> 8));
  spi_tx((uint8_t)(addr & 0xff));
  return spi_tx(0);
}
static void read_program_memory(uint16_t addr, uint8_t* buffer, uint16_t size) {
  uint16_t offset = 0;
  for (; size > 1; ++addr, size -= 2) {
    buffer[offset++] = read_program_memory_byte(READ_PROGRAM_MEMORY_LO, addr);
    buffer[offset++] = read_program_memory_byte(READ_PROGRAM_MEMORY_HI, addr);
  }
  if (size > 0) {
    buffer[offset] = read_program_memory_byte(READ_PROGRAM_MEMORY_LO, addr);
  }
}

// Write a page of program memory.
#define LOAD_PROGRAM_MEMORY_LO      0x40
#define LOAD_PROGRAM_MEMORY_HI      0x48
#define WRITE_PROGRAM_MEMORY_PAGE   0x48
static void load_program_memory_byte(uint8_t command, uint8_t offset,
                                     uint8_t value) {
  spi_tx(command);
  spi_tx(0);
  spi_tx(offset);
  spi_tx(value);
}

// Assumes |addr| is page-aligned.
static void write_program_memory_page(uint16_t addr,
                                      const uint8_t* buffer,
                                      uint16_t size) {
  // Load the program memory data to write.
  for (uint16_t offset = 0;
       offset < size && offset < PROGRAM_MEMORY_PAGE_SIZE;
       offset += 2) {
    load_program_memory_byte(LOAD_PROGRAM_MEMORY_LO, offset / 2,
                             buffer[offset]);
    load_program_memory_byte(LOAD_PROGRAM_MEMORY_HI, offset / 2,
                             buffer[offset + 1]);
  }

  // Initiate page write.
  spi_tx(WRITE_PROGRAM_MEMORY_PAGE);
  spi_tx((uint8_t)(addr >> 8));
  spi_tx((uint8_t)(addr & 0xff));
  spi_tx(0);
}

void isp_init() {
  // Deselect the ISP interface before setting the pin as an output.
  isp_release();
  ISP_DDR |= (1 << ISP_RESET_BIT);
}

uint16_t isp_program(uint16_t handle) {
  isp_enable();

  // Determine file size.
  uint32_t data_size = file_size(handle);

  // Check device ID.
  uint32_t id = read_device_id();
  if (id != EXPECTED_DEVICE_ID) {
#if defined(DEBUG)
    printf("ID: 0x%08lx, expected: 0x%08lx\n", id, EXPECTED_DEVICE_ID);
#endif  // defined(DEBUG)
    isp_release();
    return ISP_ID_MISMATCH;
  }

#if defined(DEBUG)
  printf_P(PSTR("Chip erase started.\n"));
#endif  // defined (DEBUG)
  // Start chip erase.
  chip_erase();

  // Wait for RDY = 0.
  // TODO: add timeout.
  while (!poll_ready());

#if defined(DEBUG)
  printf_P(PSTR("Chip erase complete.\n"));
#endif  // defined (DEBUG)

  // Write the new data to flash.
  const uint16_t page_size = PROGRAM_MEMORY_PAGE_SIZE;
  uint8_t file_buf[page_size];
  for (uint32_t offset = 0; offset < data_size; offset += page_size) {
    // Read the file.
    uint16_t size_read = file_read(handle, file_buf, page_size);
    if (size_read < page_size && size_read != offset - data_size) {
      return ISP_FILE_READ_ERROR;
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

    // Program a |page_size| page of the flash.
#if defined(DEBUG)
    printf_P(PSTR("Programming data at 0x%08lx\n"), offset);
#endif  // defined(DEBUG)
    write_program_memory_page(offset / 2, file_buf, size_read);

    // Wait for programming to finish.
    // TODO: add timeout.
    while (!poll_ready());

    // Read back and check the checksum.
    read_program_memory(offset / 2, file_buf, size_read);
    uint32_t new_checksum = get_checksum(file_buf, size_read);

    if (checksum != new_checksum) {
      return ISP_WRITE_VERIFY_ERROR;
    }
  }

  isp_release();

  return ISP_PROGRAM_SUCCESS;
}
