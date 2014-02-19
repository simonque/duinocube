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

#include <string.h>

#include <avr/io.h>

#include "DuinoCube_gamepad.h"
#include "FatFS/ff.h"
#include "display.h"
#include "file.h"
#include "isp.h"
#include "printf.h"
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
#define CONFIRM_X                          12
#define CONFIRM_Y          (FILE_LIST_Y + MAX_FILES_LISTED + 2)
#define STATUS_X                            4
#define STATUS_Y              (CONFIRM_Y + 4)

// Gamepad button masks.
#define SELECT_BUTTON_MASK    (1 << GAMEPAD_BUTTON_1)
#define CANCEL_BUTTON_MASK    (1 << GAMEPAD_BUTTON_2)

// File access definitions.
#define MAX_FILENAME_SIZE      13     // 8.3 format + null terminator.
#define MAX_FILES_LISTED       16     // Only list up to this many files.

// Graphics usage defs.

// Layer and palette for background image.
#define BG_LAYER_INDEX          0
#define BG_PALETTE_INDEX        0

// Layer and palette for text rendering.
#define TEXT_LAYER_INDEX        3
#define TEXT_PALETTE_INDEX      3

// Background image files.
static const char kBGImageDataFile[] PROGMEM = "data/angels.raw";
static const char kBGImagePaletteFile[] PROGMEM = "data/angels.pal";

// Background image dimensions.
#define BG_IMAGE_WIDTH        320
#define BG_IMAGE_HEIGHT       240

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

// Various message strings.
const char kProgramConfirmText[] PROGMEM = "Program with this file?";
const char kFileOpenErrorText[] PROGMEM = "Could not open file.";
const char kNotSupportedText[] PROGMEM = "Operation not yet supported.";
const char kProgrammingText[] PROGMEM = "Starting programming operation.";
const char kDoneText[] PROGMEM = "Done!";

// Store the length of the last status message so we know how much to erase.
static uint16_t last_status_message_len;

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

  display_text_render(text, MAIN_MENU_X, MAIN_MENU_Y + menu_index);
}

// Returns true if the user provided some gamepad input.
static bool has_user_input(const USB_JoystickState& input) {
  return !(input.buttons == 0 && input.x == 0 && input.y == 0);
}

// Waits for there to be no input on the gamepad, and then wait for input on the
// gamepad. Kind of like a getch() for the gamepad.
static void read_usb_gamepad(USB_JoystickState* input) {
  // First wait for user input to clear, so that nothing is being pressed on the
  // gamepad.
  do {
    usb_update();
    usb_read_joystick(input);
  } while (has_user_input(*input));
  // Then wait for something to be pressed.
  do {
    usb_update();
    usb_read_joystick(input);
  } while (!has_user_input(*input));
}

// Update cursor location.
static void move_cursor(uint16_t current_option, uint16_t new_option,
                        uint8_t menu_x, uint8_t menu_y) {
  // Erase the previous cursor and draw the new one.
  display_text_render(" ", menu_x - 2, menu_y + current_option);
  display_text_render(">", menu_x - 2, menu_y + new_option);
}

// Update the status message. Erases the previous status.
static void display_status(const char* text, bool is_progmem) {
  display_text_clear(last_status_message_len, STATUS_X, STATUS_Y);
  if (is_progmem) {
    display_text_render_P(text, STATUS_X, STATUS_Y);
    last_status_message_len = strlen_P(text);
  } else {
    display_text_render(text, STATUS_X, STATUS_Y);
    last_status_message_len = strlen(text);
  }
}

// Loads the background image.
static void load_bg_image() {
  display_bg_init(BG_LAYER_INDEX, BG_PALETTE_INDEX);

  char filename[32];
  strncpy_P(filename, kBGImagePaletteFile, sizeof(filename));
  display_bg_load_palette(filename);
  strncpy_P(filename, kBGImageDataFile, sizeof(filename));
  display_bg_load_image(filename, BG_IMAGE_WIDTH, BG_IMAGE_HEIGHT);
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
    fprintf_P(stderr, PSTR("f_opendir() returned %d\n"), result);
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
    fprintf_P(stderr, PSTR("f_readdir() returned %d\n"), result);
#endif  // defined(DEBUG)
    return result;
  }

  return result;
}

// Programs a file for the given menu selection.
void program_file(uint16_t menu_index, const char* filename) {
  switch (menu_index) {
  case MENU_LOAD_PROGRAM:
  {
    uint16_t handle = file_open(filename, FA_READ);
    if (!handle) {
      display_status(kFileOpenErrorText, true);
      break;
    }
    display_status(kProgrammingText, true);
    // TODO: Check for programming errors.
    // TODO: Have a progress bar.
    uint16_t result = isp_program(handle);
    isp_reset();    // Reset the Arduino again so it doesn't misbehave.
                    // TODO: restructure this code so it isn't necessary.
#if defined(DEBUG)
    printf_P(PSTR("isp_program() returned %d\n"), result);
#endif  // defined(DEBUG)
    display_status(kDoneText, true);
    file_close(handle);
    break;
  }
  case MENU_RUN_PROGRAM:
    // Not a programming operation.
    break;
  case MENU_BURN_BOOTLOADER:
  case MENU_UPDATE_FPGA:
  default:
    display_status(kNotSupportedText, true);
    break;
  }
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
  uint8_t num_files = 0;
  while (num_files < MAX_FILES_LISTED && strlen(filename_ptr) > 0) {
    display_text_render(filename_ptr, FILE_LIST_X, FILE_LIST_Y + num_files);
    ++num_files;
    filename_ptr += MAX_FILENAME_SIZE;
  }

  // Allow the user to select a file.
  // TODO: combine with the menu selection in boot_run();
  bool done = false;
  uint8_t file_index = 0;
  uint8_t new_file_index = 0;
  bool cursor_moved = false;
  bool file_selected = false;
  USB_JoystickState input;

  move_cursor(file_index, new_file_index, FILE_LIST_X, FILE_LIST_Y);
  while (!done) {
    read_usb_gamepad(&input);

    // Move cursor up/down.
    if (input.y < 0) {
      new_file_index = file_index ? (file_index - 1) : num_files - 1;
      cursor_moved = true;
    } else if (input.y > 0) {
      new_file_index = (file_index == num_files - 1) ? 0 : file_index + 1;
      cursor_moved = true;
    }
    if (cursor_moved) {
      // If the cursor was updated, just update the cursor on the screen.
      move_cursor(file_index, new_file_index, FILE_LIST_X, FILE_LIST_Y);
      file_index = new_file_index;
      cursor_moved = false;
      continue;
    }

    if (input.buttons & SELECT_BUTTON_MASK) {
      file_selected = true;
    } else if (input.buttons & CANCEL_BUTTON_MASK) {
      done = true;
    }

    if (file_selected) {
      display_text_render_P(kProgramConfirmText, CONFIRM_X, CONFIRM_Y);

      read_usb_gamepad(&input);
      // TODO: Use an explicit YES/NO menu.
      if (input.buttons & SELECT_BUTTON_MASK) {
        // Program!
        const char* current_filename =
            filenames + file_index * MAX_FILENAME_SIZE;
        program_file(menu_index, current_filename);
        done = true;
      }
      // Clear the confirmation text.
      display_text_clear(strlen_P(kProgramConfirmText), CONFIRM_X, CONFIRM_Y);
    }
  }

  // Clear the menu and cursor when done.
  for (uint8_t i = 0; i < num_files; ++i) {
    display_text_clear(MAX_FILENAME_SIZE, FILE_LIST_X, FILE_LIST_Y + i);
  }
  // TODO: Replace the "2" with a #define.
  display_text_clear(1, FILE_LIST_X - 2, FILE_LIST_Y + file_index);

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

  // Initialize display.
  display_init();

  // Initialize text display system.
  display_text_init(TEXT_LAYER_INDEX, TEXT_PALETTE_INDEX);

  // Load background image.
  load_bg_image();

  // Main loop variables.
  bool boot_done = false;
  uint8_t current_option = 0;   // Currently selected menu option index.
  bool cursor_moved = false;    // Set this if cursor should be updated.
  uint8_t new_option;           // Newly selected menu index.

  // Show the cursor.
  move_cursor(0, current_option, MAIN_MENU_X, MAIN_MENU_Y);

  // Main loop.
  while (!boot_done) {
    // Display main menu.
    for (uint8_t i = 0; i < NUM_MENU_OPTIONS; ++i) {
      show_main_menu_option(i);
    }

    USB_JoystickState input;
    read_usb_gamepad(&input);
    display_status("", false);   // Reset the status when the user makes a move.

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
      move_cursor(current_option, new_option, MAIN_MENU_X, MAIN_MENU_Y);
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

  // Reset the display again, so that the program will start with a clean slate.
  display_init();
}
