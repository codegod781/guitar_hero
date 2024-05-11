#include "guitar_state.h"
#include "guitar_reader.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

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

char *read_note(int guitar_fd) {
  int arg;

  if (ioctl(guitar_fd, GUITAR_READER_READ, &arg)) {
    perror("ioctl(GUITAR_READER_READ) failed");
  }

  // Static buffer to hold the string (two characters + null terminator)
  static char result_string[3];

  // Convert the integer value to a two-digit hexadecimal string
  snprintf(result_string, 3, "%02x", arg);

  return result_string;
}

char *hex_to_binary(char hex) {
  switch (hex) {
  case '0':
    return "0000";
  case '1':
    return "0001";
  case '2':
    return "0010";
  case '3':
    return "0011";
  case '4':
    return "0100";
  case '5':
    return "0101";
  case '6':
    return "0110";
  case '7':
    return "0111";
  case '8':
    return "1000";
  case '9':
    return "1001";
  case 'a':
    return "1010";
  case 'b':
    return "1011";
  case 'c':
    return "1100";
  case 'd':
    return "1101";
  case 'e':
    return "1110";
  case 'f':
    return "1111";
  default:
    return NULL;
  }
}

// Function to convert a hexadecimal string to its binary representation
char *hex_string_to_binary(const char *hex_string) {
  size_t length = strlen(hex_string);
  size_t binary_length =
      length * 4; // Each hexadecimal character represents 4 bits
  char *binary_string =
      (char *)malloc(binary_length + 1); // +1 for null terminator

  if (binary_string == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return NULL;
  }

  binary_string[binary_length] = '\0'; // Null terminate the binary string

  for (size_t i = 0; i < length; i++) {
    char *binary_digit = hex_to_binary(hex_string[i]);
    if (binary_digit == NULL) {
      free(binary_string);
      return NULL;
    }
    strcat(binary_string, binary_digit);
  }

  return binary_string;
}

void set_note_guitar(guitar_state *guitar_state, const char *binary_string) {
  if (guitar_state == NULL || binary_string == NULL) {
    return; // Error handling: Ensure note_state and binary_string are not NULL
  }

  // Convert the binary string to integer values
  int green = binary_string[7] - '0';
  int red = binary_string[6] - '0';
  int yellow = binary_string[5] - '0';
  int blue = binary_string[4] - '0';
  int orange = binary_string[3] - '0';
  int strum = binary_string[2] - '0';

  // Assign the values to the struct fields
  guitar_state->green = !green;
  guitar_state->red = !red;
  guitar_state->yellow = !yellow;
  guitar_state->blue = !blue;
  guitar_state->orange = !orange;
  guitar_state->strum = strum;
}