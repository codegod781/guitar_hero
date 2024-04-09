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

  // Set up VGA emulator. Requires libsdl2-dev
  VGAEmulator emulator;
  if (VGAEmulator_init(&emulator, framebuffer))
    return 1;

  while (emulator.running) {
    sleep(1);
    unsigned char random_char = (unsigned char)(rand() % 256);
    memset(framebuffer, random_char, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
  }

  return 0;
}
