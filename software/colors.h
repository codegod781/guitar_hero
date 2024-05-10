#ifndef COLORS_H
#define COLORS_H

typedef struct {
  unsigned char R;
  unsigned char G;
  unsigned char B;
} RGB;

typedef enum {
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE,
    // Note sprite colors
    LIGHT_GREEN,
    MIDDLE_GREEN,
    DARK_GREEN,
    LIGHT_RED,
    MIDDLE_RED,
    DARK_RED,
    LIGHT_YELLOW,
    MIDDLE_YELLOW,
    DARK_YELLOW,
    LIGHT_BLUE,
    MIDDLE_BLUE,
    DARK_BLUE,
    LIGHT_ORANGE,
    MIDDLE_ORANGE,
    DARK_ORANGE,
    COLOR_COUNT // Gives number of predefined colors
} Color;

int get_color_index(Color color); // Maps a Color to an int, or returns -1 if not found
int get_color_from_rgb(RGB color); // Maps an RGB to an int, or returns -1 if not found

extern RGB palette[COLOR_COUNT]; // Color palette

#define BACKGROUND_COLOR (RGB){0, 0, 0};

#endif /* COLORS_H */
