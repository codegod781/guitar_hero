#ifndef SPRITES_H
#define SPRITES_H
#include <png.h>

typedef struct {
  char *filename;
  unsigned char *pixel_buffer;

  int width;
  int height;
  int B_per_row;
} sprite;

typedef struct {
  sprite green;
  sprite red;
  sprite yellow;
  sprite blue;
  sprite orange;
} generated_circles;

typedef struct {
  unsigned char R;
  unsigned char G;
  unsigned char B;
  unsigned char A;
} RGBA;


#define DARK_GRAY_THRESHOLD 70
#define MIDDLE_GRAY_THRESHOLD 125
#define LIGHT_GRAY_THRESHOLD 180
#define WHITE_THRESHOLD 255
#define COLOR_SELECTION_RANGE 5

// The RGB values to replace the colors with. Key:
// white: replaces (WHITE_THRESHOLD, WHITE_THRESHOLD, WHITE_THRESHOLD) +/- COLOR_SELECTION_RANGE
// light_gray: replaces (LIGHT_GRAY_THRESHOLD, LIGHT_GRAY_THRESHOLD, LIGHT_GRAY_THRESHOLD) +/- COLOR_SELECTION_RANGE
// middle_gray: replaces (MIDDLE_GRAY_THRESHOLD, MIDDLE_GRAY_THRESHOLD, MIDDLE_GRAY_THRESHOLD) +/- COLOR_SELECTION_RANGE
// dark_gray: replaces (DARK_GRAY_THRESHOLD, DARK_GRAY_THRESHOLD, DARK_GRAY_THRESHOLD) +/- COLOR_SELECTION_RANGE
// This assumes you're providing a grayscale image and uses average_pixel().
// Design your base correctly!
typedef struct {
  RGBA white;
  RGBA light_gray;
  RGBA middle_gray;
  RGBA dark_gray;
} circle_colors;

// Load a sprite from a filename
sprite load_sprite(char *filename);
// Free remaining memory
void unload_sprite(sprite loaded_sprite);
void unload_sprites(generated_circles circles);

void sprite_for_each_pixel(sprite loaded_sprite,
                           void (*fn)(png_bytep px, int px_row, int px_col));

// Draws the loaded_sprite centered around screenX and screenY
// Considers the top left corner of the screen (0, 0);
void draw_sprite(sprite loaded_sprite, unsigned char *framebuffer, int screenX,
                 int screenY);

// Performs a deep copy of the given sprite. Does NOT copy the filename
sprite deep_copy_sprite(sprite original);

// Returns the average of the RGB values
int average_pixel(png_bytep px);
// Returns 1 if the pixel Alpha is high enough to be visible on our VGA
int pixel_visible(png_bytep px);

void print_pixel_data(png_bytep px, int px_row, int px_col);

// Uses the template information in the base circle to generate the colored
// sprites
generated_circles
generate_circles(sprite circle_base, circle_colors green_colors,
                 circle_colors red_colors, circle_colors yellow_colors,
                 circle_colors blue_colors, circle_colors orange_colors);

#endif /* SPRITES_H */