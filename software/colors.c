#include "colors.h"

int get_color_index(Color color) {
  if (color >= 0 && color < COLOR_COUNT)
    // The label itself corresponds to the index in the palette
    return color;
  else
    return -1;
}

int get_color_from_rgb(RGB color) {
  for (int i = 0; i < COLOR_COUNT; i++) {
    if (palette[i].R == color.R && palette[i].G == color.G &&
        palette[i].B == color.B) {
      return i;
    }
  }

  return -1;
}

// Gives the RGB values of each color
RGB palette[COLOR_COUNT] = {[BLACK] = {0, 0, 0},
                            [WHITE] = {255, 255, 255},
                            [RED] = {255, 0, 0},
                            [GREEN] = {0, 255, 0},
                            [BLUE] = {0, 0, 255},
                            [LIGHT_GREEN] = {20, 211, 69},
                            [MIDDLE_GREEN] = {17, 161, 50},
                            [DARK_GREEN] = {16, 162, 55},
                            [LIGHT_RED] = {211, 54, 47},
                            [MIDDLE_RED] = {154, 42, 38},
                            [DARK_RED] = {155, 41, 41},
                            [LIGHT_YELLOW] = {254, 243, 53},
                            [MIDDLE_YELLOW] = {197, 189, 26},
                            [DARK_YELLOW] = {207, 189, 61},
                            [LIGHT_BLUE] = {83, 117, 224},
                            [MIDDLE_BLUE] = {59, 89, 175},
                            [DARK_BLUE] = {64, 89, 171},
                            [LIGHT_ORANGE] = {218, 86, 43},
                            [MIDDLE_ORANGE] = {139, 53, 24},
                            [DARK_ORANGE] = {143, 55, 25}};