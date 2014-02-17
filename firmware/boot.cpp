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

#include "DuinoCube_gamepad.h"
#include "FatFS/ff.h"
#include "file.h"
#include "isp.h"
#include "text.h"
#include "usb.h"
#include "utils.h"

// Control pin definition.
#define BOOT_MODE_PIN   PIND
#define BOOT_MODE_BIT   PORTD0      // 0 = dev mode, 1 = boot mode.

#define TEXT_BUFFER_SIZE     128    // For storing text to be rendered.

// Locations of various UI items in character coordinates.
#define MAIN_MENU_X                         4
#define MAIN_MENU_Y                         4
#define FILE_LIST_X                        24
#define FILE_LIST_Y                         4

// Gamepad button masks.
#define SELECT_BUTTON_MASK    (1 << GAMEPAD_BUTTON_1)
#define CANCEL_BUTTON_MASK    (1 << GAMEPAD_BUTTON_2)

// File access definitions.
#define MAX_FILENAME_SIZE      13     // 8.3 format + null terminator.
#define MAX_FILES_LISTED       16     // Only list up to this many files.

// Menu options.
enum {
  MENU_RUN_PROGRAM,
  MENU_LOAD_PROGRAM,
  MENU_BURN_BOOTLOADER,
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

// Returns true if the user provided some gamepad input.
static bool has_user_input(const USB_JoystickState& input) {
  return !(input.buttons == 0 && input.x == 0 && input.y == 0);
}

// Update cursor location.
static void move_cursor(uint16_t current_option, uint16_t new_option) {
  // Erase the previous cursor and draw the new one.
  text_render(" ", MAIN_MENU_X - 2, MAIN_MENU_Y + current_option);
  text_render(">", MAIN_MENU_X - 2, MAIN_MENU_Y + new_option);
}

// Returns a list of filenames in |path|. The filename strings are stored in
// |filenames|, spaced at intervals of |MAX_FILENAME_SIZE|. |buf_size| is the
// allocated size of |filenames|.
static uint16_t get_filenames(const char* path, char* filenames,
                              uint16_t buf_size) {
  // Browse the root directory.
  // TODO: allow traversal of the file system
  FRESULT result;
  FILINFO file_info;
  DIR dir;

  result = f_opendir(&dir, path);
  if (result != FR_OK) {
#if defined(DEBUG)
    fprintf_P(stderr, "f_opendir() returned %d\n", result);
#endif  // defined(DEBUG)
    return result;
  }

  // Store all filenames in one char array.
  memset(filenames, 0, buf_size);
  // Iterate through the directory's files.
  uint16_t filename_offset = 0;
  for (result = f_readdir(&dir, &file_info);
       result == FR_OK && strlen(file_info.fname) > 0 &&
          filename_offset + MAX_FILENAME_SIZE < buf_size;
       result = f_readdir(&dir, &file_info)) {
    // Skip directories.
    if (file_info.fattrib & AM_DIR) {
      continue;
    }
    // Store the filename.
    strncpy(filenames + filename_offset, file_info.fname, MAX_FILENAME_SIZE);
    filename_offset += MAX_FILENAME_SIZE;
  }
  // TODO: Upgrade to latest FatFS release so f_closedir is supported.
  //f_closedir(&dir);

  if (result != FR_OK) {
#if defined(DEBUG)
    fprintf_P(stderr, "f_readdir() returned %d\n", result);
#endif  // defined(DEBUG)
    return result;
  }

  return result;
}

// Get the user to select a file from the file system.
// |menu_index| indicates the menu option that was chosen before.
static uint16_t run_file_operation(uint16_t menu_index) {
  char filenames[MAX_FILENAME_SIZE * MAX_FILES_LISTED];
  uint16_t result = get_filenames("/", filenames, sizeof(filenames));
  if (result != FR_OK) {
    return result;
  }

  // Print the directory listing.
  char* filename_ptr = filenames;
  for (uint8_t file_index = 0;
       file_index < MAX_FILES_LISTED && strlen(filename_ptr) > 0;
       ++file_index, filename_ptr += MAX_FILENAME_SIZE) {
    text_render(filename_ptr, FILE_LIST_X, FILE_LIST_Y + file_index);
  }

  return result;
}

bool boot_mode_enabled() {
  return (BOOT_MODE_PIN & (1 << BOOT_MODE_BIT));
}

/*
 * Menu goes something like this:
 *
 * Run program -> exit boot menu and let Arduino run.
 * Load program -> select file -> ask for y/n -> program -> back to menu
 * Burn bootloader -> select file -> ask for y/n -> program -> back to menu
 * Update FPGA -> select file -> ask for y/n -> program -> back to menu
 *
 */

void boot_run() {
  // Hold Arduino in reset.
  isp_reset();

  // Initialize text display system.
  text_init(0, 0);

  // Main loop variables.
  bool boot_done = false;
  uint8_t current_option = 0;   // Currently selected menu option index.
  USB_JoystickState input;      // For reading user input.
  bool cursor_moved = false;    // Set this if cursor should be updated.
  uint8_t new_option;           // Newly selected menu index.
  bool had_user_input = false;  // User just made a move.

  // Show the cursor.
  move_cursor(0, current_option);

  // Main loop.
  while (!boot_done) {
    // Display main menu.
    for (uint8_t i = 0; i < NUM_MENU_OPTIONS; ++i) {
      show_main_menu_option(i);
    }

    // Wait for user input. Alternatively, wait for user to release the gamepad
    // if there was an earlier input.
    do {
      usb_update();
      usb_read_joystick(&input);
    } while (has_user_input(input) == had_user_input);

    had_user_input = !had_user_input;

    // Move cursor up/down.
    if (input.y < 0) {
      new_option = current_option ? (current_option - 1) : NUM_MENU_OPTIONS - 1;
      cursor_moved = true;
    } else if (input.y > 0) {
      new_option =
          (current_option == NUM_MENU_OPTIONS - 1) ? 0 : current_option + 1;
      cursor_moved = true;
    }

    if (cursor_moved) {
      // If the cursor was updated, just update the cursor on the screen.
      move_cursor(current_option, new_option);
      current_option = new_option;
      cursor_moved = false;
      continue;
    }

    // Selected the current menu option.
    if (input.buttons & SELECT_BUTTON_MASK) {
      switch (current_option) {
      case MENU_RUN_PROGRAM:
        boot_done = true;
        break;
      case MENU_LOAD_PROGRAM:
      case MENU_BURN_BOOTLOADER:
      case MENU_UPDATE_FPGA:
        uint8_t result = run_file_operation(current_option);
        // TODO: If the result was an error, display an error message.
        break;
      }
    }
  }

  // Release Arduino from reset.
  isp_release();
}
