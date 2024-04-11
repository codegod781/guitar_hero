#include "global_consts.h"
#include "sprites.h"
#include "vga_emulator.h"
#include <SDL2/SDL_blendmode.h>
#include <unistd.h>

// Color definitions (hardcoded):
circle_colors green_colors = {.white = {255, 255, 255},
                              .light_gray = {0, 0, 0},
                              .middle_gray = {0, 0, 0},
                              .dark_gray = {0, 0, 0}};

circle_colors red_colors = {.white = {255, 255, 255},
                            .light_gray = {211, 54, 47},
                            .middle_gray = {154, 42, 38},
                            .dark_gray = {155, 41, 41}};

circle_colors yellow_colors = {.white = {255, 255, 255},
                               .light_gray = {0, 0, 0},
                               .middle_gray = {0, 0, 0},
                               .dark_gray = {0, 0, 0}};

circle_colors blue_colors = {.white = {255, 255, 255},
                             .light_gray = {0, 0, 0},
                             .middle_gray = {0, 0, 0},
                             .dark_gray = {0, 0, 0}};

circle_colors orange_colors = {.white = {255, 255, 255},
                               .light_gray = {0, 0, 0},
                               .middle_gray = {0, 0, 0},
                               .dark_gray = {0, 0, 0}};

int main() {
  // 32 bits/pixel = 4 B/pixel
  unsigned char *framebuffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
  memset(framebuffer, 0, WINDOW_WIDTH * WINDOW_HEIGHT * 4);

  if (framebuffer == NULL) {
    printf("Error allocating framebuffer!\n");
    return 1;
  }

  // Load necessary sprites into memory
  sprite GH_circle_base = load_sprite("sprites/GH-Circle.png");
  generated_circles circles =
      generate_circles(GH_circle_base, green_colors, red_colors, yellow_colors,
                       blue_colors, orange_colors);

  draw_sprite(GH_circle_base, framebuffer, 12, 12);
  draw_sprite(circles.red, framebuffer, WINDOW_WIDTH - 12, 12);
  // draw_sprite(circles.green, framebuffer, WINDOW_WIDTH - 12, WINDOW_HEIGHT - 12);
  // draw_sprite(circles.yellow, framebuffer, 12, WINDOW_HEIGHT - 12);

  // Set up VGA emulator. Requires libsdl2-dev
  VGAEmulator emulator;
  if (VGAEmulator_init(&emulator, framebuffer))
    return 1;

  while (emulator.running) {
    sleep(1);
  }

  VGAEmulator_destroy(&emulator);
  unload_sprite(GH_circle_base);
  // unload_sprite(circles.green);
  unload_sprite(circles.red);
  // unload_sprite(circles.yellow);
  // unload_sprite(circles.blue);
  // unload_sprite(circles.orange);

  // unload_sprite(circle_red);
  free(framebuffer);

  return 0;
}
