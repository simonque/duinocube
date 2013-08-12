#include <DuinoCube.h>
#include <SPI.h>

// Local test functions.
static void test_registers();
static void test_palettes();
static void test_sprites();
static void test_tilemaps();

void setup() {
  Serial.begin(115200);
  DC.begin();
}

void loop() {
  test_registers();
  test_palettes();
  test_sprites();
  test_tilemaps();
  Serial.print("Done testing...\n");
  while(true); 
}

#define MAX_ERRORS_PER_TEST           16

// Poor-man's hash: Generates a code based on the input value.
word get_test_value(word value) {
  return ~value ^ 0x55aa;
}

static void test_registers() {
  Serial.print("Testing registers.\n");
  const int NUM_REGISTERS = 16;

  const int main_regs_to_read[] = {
    REG_SYS_CTRL,
    REG_MEM_BANK,
    REG_OUTPUT_CTRL,
    REG_MODE_CTRL,
  };
  for (int offset = 0;
       offset < sizeof(main_regs_to_read) / sizeof(main_regs_to_read[0]);
       ++offset) {
    int reg = main_regs_to_read[offset];
    DC.writeWord(reg, get_test_value(reg));
  }

  for (int offset = 0;
       offset < sizeof(main_regs_to_read) / sizeof(main_regs_to_read[0]);
       ++offset) {
    int reg = main_regs_to_read[offset];

    word value = DC.readWord(reg);
    word expected = get_test_value(reg);
    if (value != expected) {
      Serial.print("Mismatch in register readback. ");
      Serial.print("Register [0x");
      Serial.print(reg, HEX);
      Serial.print("], expected [0x");
      Serial.print(expected, HEX);
      Serial.print("], actual [0x");
      Serial.print(value, HEX);
      Serial.print("]\n");
    }
    // Reset register.
    DC.writeWord(reg, 0);
  }
  const int tile_regs_to_read[] = {
    TILE_CTRL_0,
    TILE_DATA_OFFSET,
    TILE_EMPTY_VALUE,
    TILE_COLOR_KEY,
    TILE_OFFSET_X,
    TILE_OFFSET_Y,
  };
  for (int index = 0; index < NUM_TILE_LAYERS; ++index) {
    for (int offset = 0;
         offset < sizeof(tile_regs_to_read) / sizeof(tile_regs_to_read[0]);
         ++offset) {
      int reg = tile_regs_to_read[offset];
      DC.writeWord(TILE_LAYER_REG(index, reg), get_test_value(index + reg));
    }

    for (int offset = 0;
         offset < sizeof(tile_regs_to_read) / sizeof(tile_regs_to_read[0]);
         ++offset) {
      int reg = tile_regs_to_read[offset];

      word value = DC.readWord(TILE_LAYER_REG(index, reg));
      word expected = get_test_value(index + reg);
      if (value != expected) {
        Serial.print("Mismatch in tile register readback. ");
        Serial.print("Tile layer ");
        Serial.print(index);
        Serial.print(", Register [0x");
        Serial.print(reg, HEX);
        Serial.print("], expected [0x");
        Serial.print(expected, HEX);
        Serial.print("], actual [0x");
        Serial.print(value, HEX);
        Serial.print("]\n");
      } else {
        // Reset register.
        DC.writeWord(TILE_LAYER_REG(index, reg), 0);
      }
    }
  }
}

static void test_palettes() {
  Serial.print("Testing palettes.\n");
  struct PaletteEntry {
    byte r, g, b;
    byte padding;
    bool operator==(const PaletteEntry& entry) {
      return entry.r == r && entry.g == g && entry.b == b && entry.padding == padding;
    }
  };
  PaletteEntry palette[NUM_PALETTE_ENTRIES];

  int num_errors = 0;

  // For each palette.
  for (int index = 0; index < NUM_PALETTES; ++index) {
    // For each palette entry.
    for (int entry = 0; entry < NUM_PALETTE_ENTRIES; ++entry) {
      palette[entry].r = get_test_value(index + entry + 0);
      palette[entry].g = get_test_value(index + entry + 1);
      palette[entry].b = get_test_value(index + entry + 2);
    }
    DC.writeData(PALETTE(index), palette, sizeof(palette));

    // Read back the data.
    memset(palette, 0, sizeof(palette));
    DC.readData(PALETTE(index), palette, sizeof(palette));

    // For each palette entry.
    for (int entry = 0; entry < NUM_PALETTE_ENTRIES; ++entry) {
      PaletteEntry single_entry;
      single_entry.r = get_test_value(index + entry + 0);
      single_entry.g = get_test_value(index + entry + 1);
      single_entry.b = get_test_value(index + entry + 2);
      if (single_entry == palette[entry])
        continue;

      Serial.print("Mismatch in palette entry readback. ");
      Serial.print("Palette ");
      Serial.print(index);
      Serial.print(", Entry [0x");
      Serial.print(entry, HEX);
      Serial.print("], expected [0x");
      Serial.print(single_entry.r, HEX);
      Serial.print(", 0x");
      Serial.print(single_entry.g, HEX);
      Serial.print(", 0x");
      Serial.print(single_entry.b, HEX);
      Serial.print(", 0x");
      Serial.print(single_entry.padding, HEX);
      Serial.print("], actual [0x");
      Serial.print(palette[entry].r, HEX);
      Serial.print(", 0x");
      Serial.print(palette[entry].g, HEX);
      Serial.print(", 0x");
      Serial.print(palette[entry].b, HEX);
      Serial.print(", 0x");
      Serial.print(palette[entry].padding, HEX);
      Serial.print("]\n");
      ++num_errors;
      if (num_errors >= MAX_ERRORS_PER_TEST)
        return;
    }
  }
}

static void test_sprites() {
  Serial.print("Testing sprites.\n");
  int num_errors = 0;
  for (int index = 0; index < NUM_SPRITES; ++index) {
    for (int reg = 0; reg < NUM_SPRITE_REGS; ++reg)
      DC.writeWord(SPRITE_REG(index, reg), get_test_value(reg + index));

    for (int reg = 0; reg < NUM_SPRITE_REGS; ++reg) {
      word value = DC.readWord(SPRITE_REG(index, reg));
      DC.writeWord(SPRITE_REG(index, reg), 0);
      word expected = get_test_value(reg + index);
      if (value == expected)
        continue;

      Serial.print("Mismatch in sprite register readback. ");
      Serial.print("Sprite ");
      Serial.print(index);
      Serial.print(", Register [0x");
      Serial.print(reg, HEX);
      Serial.print("], expected [0x");
      Serial.print(expected, HEX);
      Serial.print("], actual [0x");
      Serial.print(value, HEX);
      Serial.print("]\n");

      ++num_errors;
      if (num_errors >= MAX_ERRORS_PER_TEST)
        return;
    }
  }
}

static void test_tilemaps() {
  Serial.print("Testing tilemaps.\n");
  int num_errors = 0;
  word buf[32];

  for (int index = 0; index < NUM_TILE_LAYERS; ++index) {
    for (int offset = 0; offset < TILEMAP_SIZE; offset += sizeof(buf)) {
      for(int buf_offset = 0;
          buf_offset < sizeof(buf) / sizeof(buf[0]);
          ++buf_offset) {
        buf[buf_offset] = get_test_value(index + offset + buf_offset);
      }

      // Enable memory bank 1.
      DC.writeWord(REG_MEM_BANK, TILEMAP_BANK);
      DC.writeData(TILEMAP(index) + offset, buf, sizeof(buf));

      // Read back the data.
      memset(buf, 0, sizeof(buf));
      DC.readData(TILEMAP(index) + offset, buf, sizeof(buf));
      DC.writeWord(REG_MEM_BANK, 0);
     
      for(int buf_offset = 0;
          buf_offset < sizeof(buf) / sizeof(buf[0]);
          ++buf_offset) {
        word value = buf[buf_offset];
        word expected = get_test_value(index + offset + buf_offset);
        if (value == expected)
          continue;

        Serial.print("Mismatch in tilemap readback. ");
        Serial.print("Tilemap ");
        Serial.print(index);
        Serial.print(", offset [0x");
        Serial.print(offset + buf_offset * 2, HEX);
        Serial.print("], expected [0x");
        Serial.print(expected, HEX);
        Serial.print("], actual [0x");
        Serial.print(value, HEX);
        Serial.print("]\n");
        ++num_errors;
        if (num_errors >= MAX_ERRORS_PER_TEST)
          return;
      }
    }
  }
}

