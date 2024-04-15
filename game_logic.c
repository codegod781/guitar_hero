#include "global_consts.h"
#include "sprites.h"
#include "vga_emulator.h"
#include <SDL2/SDL_blendmode.h>
#include "VGA.h"
#include <unistd.h>
#include <linux/fb.h>
#include <stdlib.h>

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

int EMULATING_VGA = 0;

int main() {
  // 32 bits/pixel = 4 B/pixel
  unsigned char *framebuffer;
  screen_info screen;
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
  }

    printf("X resolution: %d\n", screen.fb_vinfo->xres);
    printf("Y resolution: %d\n", screen.fb_vinfo->yres);

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

  // Line of notes
  draw_sprite(note_circles.green, framebuffer, WINDOW_WIDTH / 2 - 60,
              WINDOW_HEIGHT / 2 - 30);
  draw_sprite(note_circles.red, framebuffer, WINDOW_WIDTH / 2 - 30,
              WINDOW_HEIGHT / 2 - 30);
  draw_sprite(note_circles.yellow, framebuffer, WINDOW_WIDTH / 2,
              WINDOW_HEIGHT / 2 - 30);
  draw_sprite(note_circles.blue, framebuffer, WINDOW_WIDTH / 2 + 30,
              WINDOW_HEIGHT / 2 - 30);
  draw_sprite(note_circles.orange, framebuffer, WINDOW_WIDTH / 2 + 60,
              WINDOW_HEIGHT / 2 - 30);

  // Line of play indicators (released)
  draw_sprite(play_circles_released.green, framebuffer, WINDOW_WIDTH / 2 - 60,
              WINDOW_HEIGHT / 2);
  draw_sprite(play_circles_released.red, framebuffer, WINDOW_WIDTH / 2 - 30,
              WINDOW_HEIGHT / 2);
  draw_sprite(play_circles_released.yellow, framebuffer, WINDOW_WIDTH / 2,
              WINDOW_HEIGHT / 2);
  draw_sprite(play_circles_released.blue, framebuffer, WINDOW_WIDTH / 2 + 30,
              WINDOW_HEIGHT / 2);
  draw_sprite(play_circles_released.orange, framebuffer, WINDOW_WIDTH / 2 + 60,
              WINDOW_HEIGHT / 2);

  // Line of play indicators (held)
  draw_sprite(play_circles_held.green, framebuffer, WINDOW_WIDTH / 2 - 60,
              WINDOW_HEIGHT / 2 + 30);
  draw_sprite(play_circles_held.red, framebuffer, WINDOW_WIDTH / 2 - 30,
              WINDOW_HEIGHT / 2 + 30);
  draw_sprite(play_circles_held.yellow, framebuffer, WINDOW_WIDTH / 2,
              WINDOW_HEIGHT / 2 + 30);
  draw_sprite(play_circles_held.blue, framebuffer, WINDOW_WIDTH / 2 + 30,
              WINDOW_HEIGHT / 2 + 30);
  draw_sprite(play_circles_held.orange, framebuffer, WINDOW_WIDTH / 2 + 60,
              WINDOW_HEIGHT / 2 + 30);

  // Set up VGA emulator. Requires libsdl2-dev
  // VGAEmulator emulator;
  // if (VGAEmulator_init(&emulator, framebuffer))
  //   return 1;

  while (1) {
    sleep(1);
  }

  // VGAEmulator_destroy(&emulator);
  // Clear sprites
  unload_sprite(GH_circle_base);
  unload_sprites(note_circles);
  unload_sprites(play_circles_released);
  unload_sprites(play_circles_held);

  // free(framebuffer);

  return 0;
}
