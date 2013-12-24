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

// Tool to convert BMP files to raw image data and palette files for use on
// DuinoCube.

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "EasyBMP.h"

// This is the packed palette format used in DuinoCube.
struct PaletteEntry {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};

// Extracts pixel and palette info from BMP file.  Optionally linearizes tiles
// that are laid out in a planar grid in the input file.
// TODO: This function is damn big.  Split into several functions.
bool ProcessBMP(const std::string& input_file,
                int tile_width,
                int tile_height,
                const std::string& output_format,
                const std::string& output_file,
                const std::string& palette_file) {
  BMP bmp;
  if (!bmp.ReadFromFile(input_file.c_str()))
    return false;

  int i, j, x, y;

  // For converting a RGBApixel struct to a 32-bit word.
  union {
    RGBApixel pixel;
    uint32_t pixel_value;
  };

  // Setup palette.
  int bit_depth = bmp.TellBitDepth();
  // TODO: support other depths.
  if (bit_depth != 8) {
    fprintf(stderr, "Image must be 8 bits per pixel.\n");
    return false;
  }

  int palette_size = (1 << bit_depth);
  std::map<uint32_t, unsigned int> reverse_palette;
  for (i = 0; i < palette_size; ++i) {
    pixel = bmp.GetColor(i);
    if (reverse_palette.find(pixel_value) != reverse_palette.end())
      continue;
    reverse_palette[pixel_value] = i;
  }

  // Create pixel array.
  std::vector<uint32_t> pixels;
  int width = bmp.TellWidth();
  int height = bmp.TellHeight();
  pixels.resize(width * height);
  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      pixel = bmp.GetPixel(x, y);
      pixels[x + y * width] = pixel_value;
    }
  }

  // Convert pixel array from planar to linear based on tile dimensions, if they
  // are provided.
  std::vector<uint32_t> new_pixels;
  int tile_index = 0;
  if (tile_width > 0 && tile_height > 0) {
    new_pixels.resize(pixels.size());
    for (y = 0; y < height; y += tile_height) {
      for (x = 0; x < width; x += tile_width) {
        // (x, y) points to the top left corner of the tile.
        int src_offset = x + y * width;
        // Determine the destination offset for copying the tile.
        int dst_offset = tile_index * tile_width * tile_height;
        // Copy the tile line by line.
        for (j = 0; j < tile_height; ++j) {
          memcpy(&new_pixels[dst_offset],
                 &pixels[src_offset],
                 tile_width * sizeof(uint32_t));
          src_offset += width;
          dst_offset += tile_width;
        }
        ++tile_index;
      }
    }
  } else {
    new_pixels = pixels;
  }

  // Write pixel array to output file.
  // TODO: this is currently 8-bit, make it generic.
  FILE* fp = fopen(output_file.c_str(), "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open %s for writing.\n", output_file.c_str());
    return false;
  }
  for (i = 0; i < new_pixels.size(); ++i) {
    pixel_value = new_pixels[i];
    uint8_t color_index = 0;
    if (reverse_palette.find(pixel_value) != reverse_palette.end())
      color_index = reverse_palette[pixel_value];
    fwrite(&color_index, sizeof(color_index), 1, fp);
  }
  fclose(fp);

  // If tile dimensions were given, and the tile layout was linearized, write
  // the same pixel array to a "proof" bmp file so its correctness can be
  // verified visually.
  if (tile_width > 0 && tile_height > 0) {
    BMP new_bmp;
    new_bmp.SetSize(tile_width, tile_index * tile_height);
    for (y = 0; y < tile_index * tile_height; ++y) {
      for (x = 0; x < tile_width; ++x) {
        pixel_value = new_pixels[x + y * tile_width];
        new_bmp.SetPixel(x, y, pixel);
      }
    }
    if (!new_bmp.WriteToFile((output_file + ".bmp").c_str()))
      return false;
  }

  // Write palette to palette file.
  fp = fopen(palette_file.c_str(), "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open %s for writing.\n", output_file.c_str());
    return false;
  }
  std::vector<PaletteEntry> palette;
  palette.resize(palette_size);
  for (i = 0; i < palette_size; ++i) {
    pixel = bmp.GetColor(i);
    palette[i].red = pixel.Red;
    palette[i].green = pixel.Green;
    palette[i].blue = pixel.Blue;
    palette[i].alpha = pixel.Alpha;
  }
  fwrite(&palette[0], sizeof(PaletteEntry), palette_size, fp);
  fclose(fp);

  return true;
}

// Usage: bmp2raw [input file] -f rNgNbN -s WxH -o [output file] -p [palette file]
void PrintUsage() {
  printf("Usage:\n");
  printf("  bmp2raw [input_file] -s WxH -o [output_file] -p [palette_file].\n");
  printf("  -s WxH           Specify tile dimensions W x H in pixels.\n");
  printf("                   Tiles will not be linearized if not specified.\n");
  printf("  -o output_file   Specify file path for pixel data output.\n");
  printf("                   Default: \"[input_file].dat\"\n");
  printf("  -p palette_file  Specify file path for palette data output.\n");
  printf("                   Default: \"[input_file].pal\"\n");
  printf("\n");
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    PrintUsage();
    return 0;
  }

  std::string format_string = "r5g6b5";
  int tile_width = -1;
  int tile_height = -1;
  std::string output_file;
  std::string palette_file;

  int c;
  while ((c = getopt(argc, argv, "f:s:o:p:")) != -1) {
    switch(c) {
    case 'f':
      // TODO: actually use pixel format.
      format_string = optarg;
      break;
    case 's':
      if (sscanf(optarg, "%ux%u", &tile_width, &tile_height) < 2) {
        fprintf(stderr, "Unable to parse tile size: %s\n", optarg);
        return 1;
      }
      break;
    case 'o':
      output_file = optarg;
      break;
    case 'p':
      palette_file = optarg;
      break;
    default:
      fprintf(stderr, "Unrecognized option: -%c\n", c);
      return 1;
      break;
    }
  }
  if (optind == argc) {
    fprintf(stderr, "Need to provide input file.\n");
    PrintUsage();
    return 2;
  }
  std::string input_file = argv[optind];
  if (output_file.empty())
    output_file = input_file + ".raw";
  if (palette_file.empty())
    palette_file = input_file + ".pal";

  if (!ProcessBMP(input_file,
                  tile_width,
                  tile_height,
                  format_string,
                  output_file,
                  palette_file)) {
    fprintf(stderr, "Error processing bitmap file %s\n", input_file.c_str());
    return 1;
  }

  printf("Wrote raw data to %s\n", output_file.c_str());
  printf("Wrote palette data to %s\n", palette_file.c_str());

  return 0;
}
