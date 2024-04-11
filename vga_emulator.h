#ifndef VGA_EMULATOR_H
#define VGA_EMULATOR_H

#include <SDL2/SDL.h>
#include <pthread.h>

typedef struct {
  SDL_Window *window;
  SDL_Surface *surface;
  pthread_t render_thread;
  pthread_t event_thread;
  int running;
  unsigned char *framebuffer;
} VGAEmulator;

// Initialize the VGA emulator
int VGAEmulator_init(VGAEmulator *emulator, unsigned char *framebuffer);

// Destroy the VGA emulator
void VGAEmulator_destroy(VGAEmulator *emulator);

#endif /* VGA_EMULATOR_H */
