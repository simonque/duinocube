#include <DuinoCube.h>
#include <SPI.h>

// Local test functions.
static void test_registers();
static void test_palettes();
static void test_sprites();
static void test_tilemaps();
static void test_vram();

void setup() {
  Serial.begin(115200);
  DC.begin();
}

void loop() {
  test_registers();
  test_palettes();
  test_sprites();
  test_tilemaps();
  test_vram();
  printf("Done testing...\n");
  while(true); 
}

#define MAX_ERRORS_PER_TEST           16

// Poor-man's hash: Generates a code based on the input value.
word get_test_value(word value) {
  return ~value ^ 0x55aa;
}

static void test_registers() {
  printf("Testing registers.\n");
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
    DC.Core.writeWord(reg, get_test_value(reg));
  }

  for (int offset = 0;
       offset < sizeof(main_regs_to_read) / sizeof(main_regs_to_read[0]);
       ++offset) {
    int reg = main_regs_to_read[offset];

    word value = DC.Core.readWord(reg);
    word expected = get_test_value(reg);
    if (value != expected) {
      printf("Mismatch in register readback. Register [0x%x], expected [0x%x], "
             " actual [0x%x]\n", reg, expected, value);
    }
    // Reset register.
    DC.Core.writeWord(reg, 0);
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
      DC.Core.writeWord(TILE_LAYER_REG(index, reg), get_test_value(index + reg));
    }

    for (int offset = 0;
         offset < sizeof(tile_regs_to_read) / sizeof(tile_regs_to_read[0]);
         ++offset) {
      int reg = tile_regs_to_read[offset];
      word value = DC.Core.readWord(TILE_LAYER_REG(index, reg));
      word expected = get_test_value(index + reg);
      if (value != expected) {
        printf("Mismatch in tile register readback. Tile layer %d, "
               "Register [0x%x], expected [0x%x], actual [0x%x]\n",
               index, reg, expected, value);
      } else {
        // Reset register.
        DC.Core.writeWord(TILE_LAYER_REG(index, reg), 0);
      }
    }
  }
}

static void test_palettes() {
  printf("Testing palettes.\n");
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
    DC.Core.writeData(PALETTE(index), palette, sizeof(palette));

    // Read back the data.
    memset(palette, 0, sizeof(palette));
    DC.Core.readData(PALETTE(index), palette, sizeof(palette));

    // For each palette entry.
    for (int entry = 0; entry < NUM_PALETTE_ENTRIES; ++entry) {
      PaletteEntry single_entry;
      single_entry.r = get_test_value(index + entry + 0);
      single_entry.g = get_test_value(index + entry + 1);
      single_entry.b = get_test_value(index + entry + 2);
      if (single_entry == palette[entry])
        continue;

      printf("Mismatch in palette entry readback. Palette %d, Entry [0x%x]\n",
             index, entry);
/*  NOTE: This causes a printing glitch for some unknown reason.  Disabling.
      printf(" expected [0x%x, 0x%x, 0x%x, 0x%x],"
             " actual [0x%x, 0x%x, 0x%x, 0x%x]\n",
             single_entry.r, single_entry.g, single_entry.b,
             single_entry.padding,
             palette[entry].r, palette[entry].g, palette[entry].b,
             palette[entry].padding);
*/
      ++num_errors;
      if (num_errors >= MAX_ERRORS_PER_TEST)
        return;
    }
  }
}

static void test_sprites() {
  printf("Testing sprites.\n");
  int num_errors = 0;
  for (int index = 0; index < NUM_SPRITES; ++index) {
    for (int reg = 0; reg < NUM_SPRITE_REGS; ++reg)
      DC.Core.writeWord(SPRITE_REG(index, reg), get_test_value(reg + index));

    for (int reg = 0; reg < NUM_SPRITE_REGS; ++reg) {
      word value = DC.Core.readWord(SPRITE_REG(index, reg));
      DC.Core.writeWord(SPRITE_REG(index, reg), 0);
      word expected = get_test_value(reg + index);
      if (value == expected)
        continue;

      printf("Mismatch in sprite register readback. Sprite %d, Register [0x%x]"
             ", expected [0x%x], actual [0x%x]\n", index, reg, expected, value);
      ++num_errors;
      if (num_errors >= MAX_ERRORS_PER_TEST)
        return;
    }
  }
}

static void test_tilemaps() {
  printf("Testing tilemaps.\n");
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
      DC.Core.writeWord(REG_MEM_BANK, TILEMAP_BANK);
      DC.Core.writeData(TILEMAP(index) + offset, buf, sizeof(buf));

      // Read back the data.
      memset(buf, 0, sizeof(buf));
      DC.Core.readData(TILEMAP(index) + offset, buf, sizeof(buf));
      DC.Core.writeWord(REG_MEM_BANK, 0);
     
      for(int buf_offset = 0;
          buf_offset < sizeof(buf) / sizeof(buf[0]);
          ++buf_offset) {
        word value = buf[buf_offset];
        word expected = get_test_value(index + offset + buf_offset);
        if (value == expected)
          continue;

        printf("Mismatch in tilemap readback. Tilemap %d, offset [0x%x], "
               "expected [0x%x], actual [0x%x]\n", index,
               offset + buf_offset * 2, expected, value, 0, 0);
        ++num_errors;
        if (num_errors >= MAX_ERRORS_PER_TEST)
          return;
      }
    }
  }
}

static void test_vram() {
  printf("Testing VRAM.\n");

  // Enable VRAM access.
  DC.Core.writeWord(REG_SYS_CTRL, (1 << REG_SYS_CTRL_VRAM_ACCESS));

  word buf[32];
  int num_errors = 0;
  for (int bank = VRAM_BANK_BEGIN; bank < VRAM_BANK_END; ++bank) {
    for (int offset = 0; offset < VRAM_BANK_SIZE; offset += sizeof(buf)) {
      for(int buf_offset = 0;
          buf_offset < sizeof(buf) / sizeof(buf[0]);
          ++buf_offset) {
        buf[buf_offset] = get_test_value(bank + offset + buf_offset);
      }

      // Select the bank and write to it.
      DC.Core.writeWord(REG_MEM_BANK, bank);
      DC.Core.writeData(VRAM_BASE + offset, buf, sizeof(buf));

      // Read it back.
      memset(buf, 0, sizeof(buf));
      DC.Core.readData(VRAM_BASE + offset, buf, sizeof(buf));

      DC.Core.writeWord(REG_MEM_BANK, 0);

      for(int buf_offset = 0;
          buf_offset < sizeof(buf) / sizeof(buf[0]);
          ++buf_offset) {
        word value = buf[buf_offset];
        word expected = get_test_value(bank + offset + buf_offset);
        if (value == expected)
          continue;

        printf("Mismatch in VRAM readback. Bank %d, offset [0x%x], expected "
               "[0x%x], actual [0x%x]\n", bank, offset + buf_offset * 2,
               expected, value);
        ++num_errors;
        if (num_errors >= MAX_ERRORS_PER_TEST)
          return;
      }
    }
  }
}

