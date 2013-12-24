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

// Utility to generate a data header file.
// Currently tailored for Atmel AVR.

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: bin2c [filename] > [output file]\n");
    return 0;
  }

  FILE *fp = fopen(argv[1], "rb");
  if (!fp)
    return 1;

  for (int i = 0; i < strlen(argv[1]); ++i) {
    if (!isalnum(argv[1][i]))
      argv[1][i] = '_';
  }

  unsigned char buf[32];
  uint32_t *buf32 = (uint32_t*) buf;
  int n;
  int total = 0;
  printf("#include <stdint.h>\n");
  printf("#include <avr/pgmspace.h>\n");
  printf("const uint32_t %s_data32[] PROGMEM = {\n", argv[1]);
  memset(buf, 0, sizeof(buf));
  while ((n = fread(buf, 1, sizeof(buf), fp)) != 0) {
    printf("\t");
    for (int i = 0; i < (n + 3) / 4; ++i)
      printf("0x%08x,", buf32[i]);
    printf("\n");
    total += n;
    memset(buf, 0, sizeof(buf));
  }
  printf("};\n");
  printf("uint8_t* %s_data8 = (uint8_t*) %s_data32;\n", argv[1], argv[1]);
  printf("uint16_t* %s_data16 = (uint16_t*) %s_data32;\n", argv[1], argv[1]);

  for (int i = 0; i < strlen(argv[1]); ++i)
    argv[1][i] = toupper(argv[1][i]);
  printf("const int %s_DATA_SIZE = %u;\n", argv[1], total);
  fclose(fp);

  return 0;
}
