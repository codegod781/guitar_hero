#include "global_consts.h"
#include "sprites.h"
#include "vga_emulator.h"
#include <unistd.h>

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

  draw_sprite(GH_circle_base, framebuffer, 12, 12);

  // Set up VGA emulator. Requires libsdl2-dev
  VGAEmulator emulator;
  if (VGAEmulator_init(&emulator, framebuffer))
    return 1;


  while (emulator.running) {
    sleep(1);
  }

  unload_sprite(GH_circle_base);
  free(framebuffer);

  return 0;
}
