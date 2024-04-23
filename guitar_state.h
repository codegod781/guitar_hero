#ifndef GUITAR_STATE_H
#define GUITAR_STATE_H

typedef struct {
  int green;
  int red;
  int yellow;
  int blue;
  int orange;
  int strum_bar_down;
} guitar_state;

void init_guitar_state(guitar_state *gs);
#endif
