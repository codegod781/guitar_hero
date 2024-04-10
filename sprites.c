#include "sprites.h"
#include <stdlib.h>
#include <string.h>

// Adapted from https://gist.github.com/niw/5963798
sprite load_sprite(char *filename) {
  sprite loaded_sprite;
  FILE *fp = fopen(filename, "rb");

  png_structp png =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    perror("png_create_read_struct() encountered a fatal error!");
    exit(1);
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    perror("png_create_info_struct() encountered a fatal error!");
    exit(1);
  }

  if (setjmp(png_jmpbuf(png))) {
    perror("setjmp() encountered a fatal error!");
    exit(1);
  }

  png_init_io(png, fp);

  png_read_info(png, info);

  loaded_sprite.width = png_get_image_width(png, info);
  loaded_sprite.height = png_get_image_height(png, info);
  png_byte color_type = png_get_color_type(png, info);
  png_byte bit_depth = png_get_bit_depth(png, info);

  if (bit_depth == 16)
    png_set_strip_16(png);

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  loaded_sprite.B_per_row = png_get_rowbytes(png, info);

  png_bytep *row_pointers =
      (png_bytep *)malloc(sizeof(png_bytep) * loaded_sprite.height);

  for (int y = 0; y < loaded_sprite.height; y++) {
    row_pointers[y] = (png_byte *)malloc(loaded_sprite.B_per_row);
  }

  png_read_image(png, row_pointers);

  loaded_sprite.pixel_buffer =
      malloc(loaded_sprite.height * loaded_sprite.B_per_row);

  printf("\n\n\n");

  for (int y = 0; y < loaded_sprite.height; y++) {
    memcpy(loaded_sprite.pixel_buffer + y * loaded_sprite.B_per_row * 4,
           row_pointers[y], loaded_sprite.B_per_row);
    free(row_pointers[y]);
  }

  free(row_pointers);

  fclose(fp);
  png_destroy_read_struct(&png, &info, NULL);

  return loaded_sprite;
}

void unload_sprite(sprite loaded_sprite) { free(loaded_sprite.pixel_buffer); }

void sprite_for_each_pixel(sprite loaded_sprite,
                           void (*fn)(png_bytep px, int px_row, int px_col)) {
  for (int y = 0; y < loaded_sprite.height; y++) {
    for (int x = 0; x < loaded_sprite.width; x++) {
      png_bytep px = &(
          loaded_sprite.pixel_buffer[y * loaded_sprite.B_per_row * 4 + x * 4]);
      fn(px, y, x);
    }
  }
}

int average_pixel(png_bytep px) {
    return (px[0] + px[1] + px[2]) / 3;
}

int pixel_visible(png_bytep px) {
    return px[3] >= 127; // Our VGA doesn't support transparency anyways
}

// For debugging purposes
void print_pixel_data(png_bytep px, int px_row, int px_col) {
  printf("Got pixel: row %d, col %d = RGBA(%3d, %3d, %3d, %3d)\n", px_row, px_col, px[0],
         px[1], px[2], px[3]);
}

