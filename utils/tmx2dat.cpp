// Copyright (C) 2013 Simon Que
//
// This file is part of ChronoCube.
//
// ChronoCube is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ChronoCube is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with ChronoCube.  If not, see <http://www.gnu.org/licenses/>.

// TMX converter that generates raw tile layer info from TMX file.
// For info on the TMX file format, see:
//   https://github.com/bjorn/tiled/wiki/TMX-Map-Format

// This application depends on TmxParser:
//   https://code.google.com/p/tmx-parser/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <vector>

#include <TmxParser/Tmx.h>

// Default format for ChronoCube.
// TODO: make it expandable to different formats.
struct TileInfo {
  unsigned int index:13;
  bool hflip:1;
  bool vflip:1;
  bool dflip:1;
};

namespace {

// Rounds a value up to the next power of two, if it is not already one.  e.g:
//     0 ->  1
//     1 ->  1
//     2 ->  2
//     3 ->  4
//     4 ->  4
//     5 ->  8
//    10 -> 16
//    23 -> 32
unsigned int round_up_to_power_of_2(unsigned int value) {
    unsigned int i;
    value -= 1;
    for (i = 0; value > 0; ++i)
        value >>= 1;
    return 1 << i;
}

void PrintUsage() {
  printf("Usage:\n");
  printf("  tmx2dat -a [input_file].\n");
  printf("Options:\n");
  printf("  -a       Align the width to a power of 2.\n");
  printf("           Default: false\n");
  printf("\n");
}

const int kNopTileValue = 0x1fff;

}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 2) {
    PrintUsage();
    return 0;
  }

  int c;
  bool do_align = false;
  while ((c = getopt(argc, argv, "a")) != -1) {
    switch(c) {
    case 'a':
      do_align = true;
      break;
    default:
      fprintf(stderr, "Unrecognized option: -%c\n", c);
      return 1;
      break;
    }
  }

  Tmx::Map map;
  const char* tmx_filename = argv[optind];
  map.ParseFile(tmx_filename);

  if (map.HasError()) {
    fprintf(stderr,
            "TMX Parser encountered error [%d]: %s\n",
            map.GetErrorCode(),
            map.GetErrorText().c_str());
    return map.GetErrorCode();
  }

  // Open resource file for writing.
  char res_filename[1024];
  sprintf(res_filename, "%s.res", tmx_filename);
  FILE* res_file = fopen(res_filename, "w");
  assert(res_file);

  std::vector<std::vector<uint16_t> > layers;
  layers.resize(map.GetNumLayers());

  // Iterate through the layers.
  for (int i = 0; i < map.GetNumLayers(); ++i) {
    // Get a layer.
    const Tmx::Layer& layer = *map.GetLayer(i);
    int layer_width = layer.GetWidth();
    int layer_height = layer.GetHeight();
    if (layer_width <= 0 || layer_height <= 0) {
      fprintf(stderr, "Invalid layer dimensions %d x %d.\n", layer_width,
              layer_height);
      fclose(res_file);
      return 2;
    }

    layers[i].resize(layer_width * layer_height);
    int j = 0;
    for (int y = 0; y < layer_height; ++y) {
      for (int x = 0; x < layer_width; ++x) {
        TileInfo& info = *((TileInfo*)&layers[i][j++]);
        info.hflip = layer.IsTileFlippedHorizontally(x, y);
        info.vflip = layer.IsTileFlippedVertically(x, y);
        info.dflip = layer.IsTileFlippedDiagonally(x, y);
        if (layer.GetTileTilesetIndex(x, y) < 0)
          info.index = kNopTileValue;
        else
          info.index = layer.GetTileId(x, y);
      }
    }

    // Write layer data to file.
    // Make sure to align to a width that is a power of 2.
    unsigned int aligned_layer_width = do_align ?
                                       round_up_to_power_of_2(layer_width) :
                                       layer_width;
    char map_filename[1024];
    sprintf(map_filename, "%s.layer%0d.dat", tmx_filename, i);
    FILE *layer_file = fopen(map_filename, "wb");
    unsigned int offset;
    std::vector<uint16_t> layer_line;
    for (offset = 0; offset < layers[i].size(); offset += layer_width) {
      layer_line.assign(layers[i].begin() + offset,
                        layers[i].begin() + offset + layer_width);
      layer_line.resize(aligned_layer_width, 0);
      fwrite(&layer_line[0], sizeof(uint16_t), aligned_layer_width, layer_file);
    }
    fclose(layer_file);
    printf("Wrote layer %d data to %s\n", i, map_filename);

    // Write layer info to resource file.
    fprintf(res_file, "[%s]\n", map_filename);
    fprintf(res_file, "width=%u\n", aligned_layer_width);
    fprintf(res_file, "height=%u\n", layer_height);
    fprintf(res_file, "nop_value=0x%x\n", kNopTileValue);
    fprintf(res_file, "\n");
  }
  printf("Wrote resource info to %s\n", res_filename);
  fclose(res_file);

  return 0;
}
