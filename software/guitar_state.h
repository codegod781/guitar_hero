#ifndef GUITAR_STATE_H
#define GUITAR_STATE_H

typedef struct {
  int green;
  int red;
  int yellow;
  int blue;
  int orange;
  int strum;
} guitar_state;

void init_guitar_state(guitar_state *gs);

char *read_note(int guitar_fd);
char *hex_to_binary(char hex);
char *hex_string_to_binary(const char *hex_string);
void set_note_guitar(guitar_state *guitar_state, const char *binary_string);
#endif
