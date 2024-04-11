#include "vga_emulator.h"
#include "global_consts.h"
#include <unistd.h>

void *render(void *args) {
  VGAEmulator *emulator = (VGAEmulator *)args;
  SDL_Surface *surface = emulator->surface;
  unsigned char *framebuffer = emulator->framebuffer;

  while (emulator->running) {
    for (int y = 0; y < WINDOW_HEIGHT; ++y) {
      for (int x = 0; x < WINDOW_WIDTH; ++x) {
        unsigned char *pixel = framebuffer + (y * WINDOW_WIDTH + x) * 4;

        Uint8 r = pixel[2];
        Uint8 g = pixel[1];
        Uint8 b = pixel[0];

        Uint32 color = SDL_MapRGB(surface->format, r, g, b);
        SDL_Rect pixel_rect = {x, y, 1, 1};

        SDL_FillRect(surface, &pixel_rect, color);
      }
    }
    SDL_UpdateWindowSurface(emulator->window);
    usleep(16667); // 60 Hz refresh rate
  }
  return NULL;
}

void *handle_events(void *args) {
  VGAEmulator *emulator = (VGAEmulator *)args;
  SDL_Event event;
  while (1) {
    while (emulator->running && SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        VGAEmulator_destroy(emulator);
        return NULL;
      }
    }
  }
}

int VGAEmulator_init(VGAEmulator *emulator, unsigned char *framebuffer) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  emulator->window = SDL_CreateWindow("VGA Emulator", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                                      WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (emulator->window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    exit(1);
  }

  // Get the window's surface
  emulator->surface = SDL_GetWindowSurface(emulator->window);

  emulator->framebuffer = framebuffer;
  emulator->running = 1;

  if (pthread_create(&emulator->render_thread, NULL, render, emulator) != 0) {
    printf("Error creating render thread\n");
    return 2;
  }

  if (pthread_create(&emulator->event_thread, NULL, handle_events, emulator) !=
      0) {
    printf("Error creating event handling thread\n");
    return 2;
  }

  return 0;
}

void VGAEmulator_destroy(VGAEmulator *emulator) {
  emulator->running = 0;

  // Wait for the threads to finish
  pthread_join(emulator->render_thread, NULL);
  pthread_join(emulator->event_thread, NULL);

  // Clean up SDL resources
  SDL_DestroyWindow(emulator->window);
  SDL_Quit();
}
