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

// Boot controller functions.

#include "boot.h"

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "file.h"
#include "isp.h"
#include "text.h"
#include "utils.h"

// Control pin definition.
#define BOOT_MODE_PIN   PIND
#define BOOT_MODE_BIT   PORTD0      // 0 = dev mode, 1 = boot mode.

#define TEXT_BUFFER_SIZE     128    // For storing text to be rendered.

// Locations of various UI items in character coordinates.
#define MAIN_MENU_X                         4
#define MAIN_MENU_Y                         4

// Menu options.
enum {
  MENU_RUN_PROGRAM,
  MENU_LOAD_PROGRAM,
  MENU_LOAD_BOOTLOADER,
  MENU_UPDATE_FPGA,
  NUM_MENU_OPTIONS,
};

// Strings for the menu. For simplicity, define it as one long string with null
// terminators separating each string. The order must match the order of the
// enums above.
const char kMenuStrings[] PROGMEM = {
  "Run program\0"
  "Load program\0"
  "Load bootloader\0"
  "Update FPGA\0"
};

// Given a sequence of contiguous null-terminated strings, indicated by
// |string|, returns the Nth string where N = |menu_index|.
static const char* get_substring(const char* string, uint16_t menu_index) {
  for (uint16_t index = 0; index < menu_index; ++index) {
    // Increment the string pointer by the length of the current string (plus
    // null terminator) to get the next one.
    string += strlen_P(string) + 1;
  }
  return string;
}

// Draws the menu text for the given menu index option.
static void show_main_menu_option(uint16_t menu_index) {
  // Read the menu text.
  char text[TEXT_BUFFER_SIZE];
  strcpy_P(text, get_substring(kMenuStrings, menu_index));

  text_render(text, MAIN_MENU_X, MAIN_MENU_Y + menu_index);
}

bool boot_mode_enabled() {
  return (BOOT_MODE_PIN & (1 << BOOT_MODE_BIT));
}

void boot_run() {
  // Hold Arduino in reset.
  isp_reset();

  // Initialize text display system.
  text_init(0, 0);

  // Main loop.
  bool boot_done = false;
  while (!boot_done) {
    // Display main menu.
    for (uint8_t i = 0; i < NUM_MENU_OPTIONS; ++i) {
      show_main_menu_option(i);
    }

    // TODO: Create an actual menu system.
  }

  // Release Arduino from reset.
  isp_release();
}
