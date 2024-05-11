#include "guitar_state.h"
#include <pthread.h>

pthread_t keyboard_thread;
int RUNNING = 1;

void init_guitar_state(guitar_state *gs) {
  gs->green = 0;
  gs->red = 0;
  gs->yellow = 0;
  gs->blue = 0;
  gs->orange = 0;
  gs->strum = 0;
}
