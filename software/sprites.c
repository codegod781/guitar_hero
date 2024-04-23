#include "sprites.h"
#include "global_consts.h"
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
    exit(EXIT_FAILURE);
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    perror("png_create_info_struct() encountered a fatal error!");
    exit(EXIT_FAILURE);
  }

  if (setjmp(png_jmpbuf(png))) {
    perror("setjmp() encountered a fatal error!");
    exit(EXIT_FAILURE);
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
      malloc(loaded_sprite.height * loaded_sprite.B_per_row * 4);

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

sprite deep_copy_sprite(sprite original) {
  sprite copy;

  copy.pixel_buffer = malloc(original.height * original.B_per_row * 4);
  if (copy.pixel_buffer == NULL) {
    // Handle memory allocation error
    perror("Error allocating memory for pixel buffer");
    exit(EXIT_FAILURE);
  }

  // Copy the pixel data
  memcpy(copy.pixel_buffer, original.pixel_buffer,
         original.height * original.B_per_row * 4);

  // Copy other fields
  copy.width = original.width;
  copy.height = original.height;
  copy.B_per_row = original.B_per_row;

  return copy;
}

void unload_sprite(sprite loaded_sprite) { free(loaded_sprite.pixel_buffer); }
void unload_sprites(generated_circles circles) {
  unload_sprite(circles.green);
  unload_sprite(circles.red);
  unload_sprite(circles.yellow);
  unload_sprite(circles.blue);
  unload_sprite(circles.orange);
}

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

void draw_sprite(sprite loaded_sprite, unsigned char *framebuffer, int screenX,
                 int screenY) {
  // Determine the coordinates of the top left corner of the sprite on the
  // screen
  int tl[] = {screenX - loaded_sprite.width / 2,
              screenY - loaded_sprite.height / 2};

  for (int sprite_row = 0; sprite_row < loaded_sprite.height; sprite_row++) {
    for (int sprite_col = 0; sprite_col < loaded_sprite.width; sprite_col++) {
      png_bytep px = &(
          loaded_sprite.pixel_buffer[sprite_row * loaded_sprite.B_per_row * 4 +
                                     sprite_col * 4]);

      unsigned char R = (unsigned char)px[0], G = (unsigned char)px[1],
                    B = (unsigned char)px[2];

      if (!pixel_visible(px))
        continue;

      // Determine the offset of the framebuffer for this pixel
      int screen_x = tl[0] + sprite_col;
      int screen_y = tl[1] + sprite_row;

      if (screen_x < 0 || screen_y < 0)
        continue;

      long long framebuffer_offset = screen_y * SCREEN_LINE_LENGTH + screen_x * 4;

      if (framebuffer_offset < 0 ||
          framebuffer_offset >= WINDOW_WIDTH * WINDOW_HEIGHT * 4 ||
          screen_x < 0 || screen_x >= WINDOW_WIDTH || screen_y < 0 ||
          screen_y >= WINDOW_HEIGHT)
        continue;

      // printf("Drawing (R: %d, G: %d, B: %d) at screen coords (%d, %d), png "
            //  "coords (%d, %d)\n",
            //  R, G, B, screen_x, screen_y, sprite_col, sprite_row);

      // Set R, G, B
      (framebuffer + framebuffer_offset)[2] = R;
      (framebuffer + framebuffer_offset)[1] = G;
      (framebuffer + framebuffer_offset)[0] = B;
    }
  }
}

// Modifies in-place, so the return is only needed to condense generate_circles
sprite color_circle(sprite circle_base, circle_colors colors) {
  for (int y = 0; y < circle_base.height; y++) {
    for (int x = 0; x < circle_base.width; x++) {
      png_bytep px =
          &(circle_base.pixel_buffer[y * circle_base.B_per_row * 4 + x * 4]);

      int pixel_avg = average_pixel(px);
      RGBA pixel_color;

      if (WHITE_THRESHOLD - COLOR_SELECTION_RANGE <= pixel_avg &&
          pixel_avg <= WHITE_THRESHOLD + COLOR_SELECTION_RANGE)
        pixel_color = colors.white;
      else if (DARK_GRAY_THRESHOLD - COLOR_SELECTION_RANGE <= pixel_avg &&
               pixel_avg <= DARK_GRAY_THRESHOLD + COLOR_SELECTION_RANGE)
        pixel_color = colors.dark_gray;
      else if (MIDDLE_GRAY_THRESHOLD - COLOR_SELECTION_RANGE <= pixel_avg &&
               pixel_avg <= MIDDLE_GRAY_THRESHOLD + COLOR_SELECTION_RANGE)
        pixel_color = colors.middle_gray;
      else if (LIGHT_GRAY_THRESHOLD - COLOR_SELECTION_RANGE <= pixel_avg &&
               pixel_avg <= LIGHT_GRAY_THRESHOLD + COLOR_SELECTION_RANGE)
        pixel_color = colors.light_gray;
      else
        continue; // We will not modify this pixel

      // Update pixel color from template
      px[0] = pixel_color.R;
      px[1] = pixel_color.G;
      px[2] = pixel_color.B;
      px[3] = pixel_color.A;
    }
  }

  return circle_base;
}

generated_circles
generate_circles(sprite circle_base, circle_colors green_colors,
                 circle_colors red_colors, circle_colors yellow_colors,
                 circle_colors blue_colors, circle_colors orange_colors) {
  generated_circles circles;

  // Make deep copies of the original circle sprite for each color & converts
  circles.green = color_circle(deep_copy_sprite(circle_base), green_colors);
  circles.red = color_circle(deep_copy_sprite(circle_base), red_colors);
  circles.yellow = color_circle(deep_copy_sprite(circle_base), yellow_colors);
  circles.blue = color_circle(deep_copy_sprite(circle_base), blue_colors);
  circles.orange = color_circle(deep_copy_sprite(circle_base), orange_colors);

  return circles;
}

int average_pixel(png_bytep px) { return (px[0] + px[1] + px[2]) / 3; }

int pixel_visible(png_bytep px) {
  return px[3] >= 127; // Our VGA doesn't support transparency anyways
}

// For debugging purposes
void print_pixel_data(png_bytep px, int px_row, int px_col) {
  printf("Got pixel: row %d, col %d = RGBA(%3d, %3d, %3d, %3d)\n", px_row,
         px_col, px[0], px[1], px[2], px[3]);
}
