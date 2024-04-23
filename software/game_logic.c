#include "VGA.h"
#include "global_consts.h"
#include "guitar_state.h"
#include "sprites.h"
#include "vga_emulator.h"
#include <SDL2/SDL_blendmode.h>
#include <linux/fb.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

// Color definitions (hardcoded).
// Inspired by https://oaksstudio.itch.io/guitarheroui, recreated from scratch
circle_colors green_colors = {.white = {255, 255, 255, 255},
                              .light_gray = {20, 211, 69, 255},
                              .middle_gray = {17, 161, 50, 255},
                              .dark_gray = {16, 162, 55, 255}};
circle_colors red_colors = {.white = {255, 255, 255, 255},
                            .light_gray = {211, 54, 47, 255},
                            .middle_gray = {154, 42, 38, 255},
                            .dark_gray = {155, 41, 41, 255}};
circle_colors yellow_colors = {.white = {255, 255, 255, 255},
                               .light_gray = {254, 243, 53, 255},
                               .middle_gray = {197, 189, 26, 255},
                               .dark_gray = {207, 189, 61, 255}};
circle_colors blue_colors = {.white = {255, 255, 255, 255},
                             .light_gray = {83, 117, 224, 255},
                             .middle_gray = {59, 89, 175, 255},
                             .dark_gray = {64, 89, 171, 255}};
circle_colors orange_colors = {.white = {255, 255, 255, 255},
                               .light_gray = {218, 86, 43, 255},
                               .middle_gray = {139, 53, 24, 255},
                               .dark_gray = {143, 55, 25, 255}};

struct {
  int green;
  int red;
  int yellow;
  int blue;
  int orange;
} color_cols_x = {15, 45, 75, 105, 135};

int EMULATING_VGA = 1;
int SCREEN_LINE_LENGTH;

int main() {
  // 32 bits/pixel = 4 B/pixel
  unsigned char *framebuffer, *next_frame;
  screen_info screen;
  VGAEmulator emulator;

  guitar_state controller_state;
  init_guitar_state(&controller_state);

  if (EMULATING_VGA) {
    printf("Running in VGA EMULATION MODE\n");
    if ((framebuffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4)) == NULL) {
      perror("Error allocating framebuffer!\n");
      return 1;
    }
    SCREEN_LINE_LENGTH = WINDOW_WIDTH * 4;
  } else {
    framebuffer = fbopen();
    screen = get_fb_screen_info();

    SCREEN_LINE_LENGTH = screen.fb_finfo->line_length;

    printf("X resolution: %d\n", screen.fb_vinfo->xres);
    printf("Y resolution: %d\n", screen.fb_vinfo->yres);
  }

  if ((next_frame = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4)) == NULL) {
    perror("Error allocating next_frame!\n");
    return 1;
  }

  // Set black background by default
  memset(framebuffer, 0, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
  // Load necessary sprites into memory
  sprite GH_circle_base = load_sprite("sprites/GH-Circle.png");
  // Generate the sprites for the notes
  generated_circles note_circles =
      generate_circles(GH_circle_base, green_colors, red_colors, yellow_colors,
                       blue_colors, orange_colors);
  // Generate the sprites for the indicators to play:
  green_colors.white.A = 0;
  red_colors.white.A = 0;
  yellow_colors.white.A = 0;
  blue_colors.white.A = 0;
  orange_colors.white.A = 0;
  generated_circles play_circles_released =
      generate_circles(GH_circle_base, green_colors, red_colors, yellow_colors,
                       blue_colors, orange_colors);
  green_colors.white = green_colors.dark_gray;
  red_colors.white = red_colors.dark_gray;
  yellow_colors.white = yellow_colors.dark_gray;
  blue_colors.white = blue_colors.dark_gray;
  orange_colors.white = orange_colors.dark_gray;
  generated_circles play_circles_held =
      generate_circles(GH_circle_base, green_colors, red_colors, yellow_colors,
                       blue_colors, orange_colors);

  // Set up VGA emulator. Requires libsdl2-dev

  if (EMULATING_VGA)
    if (VGAEmulator_init(&emulator, framebuffer, &controller_state))
      return 1;

  while (1) {
    // Fresh start
    memset(next_frame, 0, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    // Draw the Guitar state line
    draw_sprite(controller_state.green ? play_circles_held.green
                                       : play_circles_released.green,
                next_frame, color_cols_x.green, WINDOW_HEIGHT - 12);
    draw_sprite(controller_state.red ? play_circles_held.red
                                     : play_circles_released.red,
                next_frame, color_cols_x.red, WINDOW_HEIGHT - 12);
    draw_sprite(controller_state.yellow ? play_circles_held.yellow
                                        : play_circles_released.yellow,
                next_frame, color_cols_x.yellow, WINDOW_HEIGHT - 12);
    draw_sprite(controller_state.blue ? play_circles_held.blue
                                      : play_circles_released.blue,
                next_frame, color_cols_x.blue, WINDOW_HEIGHT - 12);
    draw_sprite(controller_state.orange ? play_circles_held.orange
                                        : play_circles_released.orange,
                next_frame, color_cols_x.orange, WINDOW_HEIGHT - 12);

    // Push next frame to buffer
    memcpy(framebuffer, next_frame, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
  }

  if (EMULATING_VGA)
    VGAEmulator_destroy(&emulator);
  // Clear sprites
  unload_sprite(GH_circle_base);
  unload_sprites(note_circles);
  unload_sprites(play_circles_released);
  unload_sprites(play_circles_held);

  if (EMULATING_VGA)
    free(framebuffer);

  return 0;
}
