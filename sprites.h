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

// Load a sprite from a filename
sprite load_sprite(char *filename);
// Free remaining memory
void unload_sprite(sprite loaded_sprite);

void sprite_for_each_pixel(sprite loaded_sprite,
                           void (*fn)(png_bytep px, int px_row, int px_col));

// Draws the loaded_sprite centered around screenX and screenY
// Considers the top left corner of the screen (0, 0);
void draw_sprite(sprite loaded_sprite, unsigned char *framebuffer, int screenX, int screenY);

// Returns the average of the RGB values
int average_pixel(png_bytep px);
// Returns 1 if the pixel Alpha is high enough to be visible on our VGA
int pixel_visible(png_bytep px);

void print_pixel_data(png_bytep px, int px_row, int px_col);

#endif /* SPRITES_H */