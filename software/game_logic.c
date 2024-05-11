#include "colors.h"
#include "global_consts.h"
#include "guitar_reader.h"
#include "guitar_state.h"
#include "guitar_reader.h"
#include "song_data.h"
#include "sprites.h"
#include "vga_emulator.h"
#include "vga_framebuffer.h"
#include <SDL2/SDL_blendmode.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <math.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int EMULATING_VGA = 0;

int SCREEN_LINE_LENGTH;
int vga_framebuffer_fd, guitar_fd;
unsigned char *framebuffer;
pthread_mutex_t framebuffer_mutex = PTHREAD_MUTEX_INITIALIZER,
                controller_mutex = PTHREAD_MUTEX_INITIALIZER;
guitar_state controller_state;

struct {
  int green;
  int red;
  int yellow;
  int blue;
  int orange;
} color_cols_x = {15, 45, 75, 105, 135};

char *read_note() {
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

void set_note_guitar(guitar_state *note_state, const char *binary_string) {
  if (note_state == NULL || binary_string == NULL) {
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
  note_state->green = !green;
  note_state->red = !red;
  note_state->yellow = !yellow;
  note_state->blue = !blue;
  note_state->orange = !orange;
  note_state->strum = strum;
}

void *read_and_buffer_input(void *arg) {
  (void) arg; // Suppress unused warning

  while (1) {
    char *guitar_hex = read_note();
    char *binary_string = hex_string_to_binary(guitar_hex);

    guitar_state note;
    set_note_guitar(&note, binary_string);

    pthread_mutex_lock(&controller_mutex);
    controller_state = note;
    pthread_mutex_unlock(&controller_mutex);

    usleep(16667); // 60 Hz refresh rate
  }
  return NULL;
}

long long current_time_in_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long long ms =
      (tv.tv_sec * 1000LL) +
      (tv.tv_usec /
       1000); // Convert seconds to ms and add microseconds converted to ms
  return ms;
}

uint32_t pixel_writedata(unsigned char pixel_color, int pixel_row,
                         int pixel_col) {
  uint32_t pixel_writedata;
  // Make pixel_color pixel_writedata fits into 6 bits
  pixel_color &= 0x3F;

  // Make sure pixel_col fits into 8 bits
  pixel_col &= 0xFF;

  // Make sure pixel_row fits into 9 bits
  pixel_row &= 0x1FF;

  // Combine the values
  pixel_writedata = 0;
  pixel_writedata |= (uint32_t)pixel_color;       // 6 least significant bits
  pixel_writedata |= ((uint32_t)pixel_col << 6);  // Next 8 bits
  pixel_writedata |= ((uint32_t)pixel_row << 14); // Next 9 bits

  return pixel_writedata;
}

void *update_framebuffer(void *arg) {
  (void)arg; // Suppress warning

  while (1) {
    pthread_mutex_lock(&framebuffer_mutex);
    for (int pixel_row = 0; pixel_row < WINDOW_HEIGHT; pixel_row++) {
      for (int pixel_col = 0; pixel_col < WINDOW_WIDTH; pixel_col++) {
        unsigned char *pixel =
            framebuffer + (pixel_row * WINDOW_WIDTH + pixel_col) * 4;
        RGB pixel_rgb = {pixel[2], pixel[1], pixel[0]};

        vga_framebuffer_arg_t vfba;
        vfba.pixel_writedata = pixel_writedata(get_color_from_rgb(pixel_rgb),
                                               pixel_row, pixel_col);
        ;

        if (ioctl(vga_framebuffer_fd, VGA_FRAMEBUFFER_UPDATE, &vfba)) {
          perror("ioctl(VGA_FRAMEBUFFER_UPDATE) failed");
        }
      }
    }
    pthread_mutex_unlock(&framebuffer_mutex);
  }
  return NULL;
}

void set_note(note_row *note_state, const char *binary_string) {
  if (note_state == NULL || binary_string == NULL) {
    return; // Error handling: Ensure note_state and binary_string are not NULL
  }

  // Convert the binary string to integer values
  int green = binary_string[7] - '0';
  int red = binary_string[6] - '0';
  int yellow = binary_string[5] - '0';
  int blue = binary_string[4] - '0';
  int orange = binary_string[3] - '0';

  // Assign the values to the struct fields
  note_state->green = green;
  note_state->red = red;
  note_state->yellow = yellow;
  note_state->blue = blue;
  note_state->orange = orange;
}

int hit_notes(guitar_state controller_state, note_row notes) {
  return controller_state.green == notes.green &&
         controller_state.red == notes.red &&
         controller_state.yellow == notes.yellow &&
         controller_state.blue == notes.blue &&
         controller_state.orange == notes.orange;
}

int main() {
  // Color definitions (hardcoded).
  // Inspired by https://oaksstudio.itch.io/guitarheroui, recreated from scratch
  circle_colors green_colors = {.white = palette[WHITE],
                                .light_gray = palette[LIGHT_GREEN],
                                .middle_gray = palette[MIDDLE_GREEN],
                                .dark_gray = palette[DARK_GREEN]};

  circle_colors red_colors = {.white = palette[WHITE],
                              .light_gray = palette[LIGHT_RED],
                              .middle_gray = palette[MIDDLE_RED],
                              .dark_gray = palette[DARK_RED]};

  circle_colors yellow_colors = {.white = palette[WHITE],
                                 .light_gray = palette[LIGHT_YELLOW],
                                 .middle_gray = palette[MIDDLE_YELLOW],
                                 .dark_gray = palette[DARK_YELLOW]};

  circle_colors blue_colors = {.white = palette[WHITE],
                               .light_gray = palette[LIGHT_BLUE],
                               .middle_gray = palette[MIDDLE_BLUE],
                               .dark_gray = palette[DARK_BLUE]};

  circle_colors orange_colors = {.white = palette[WHITE],
                                 .light_gray = palette[LIGHT_ORANGE],
                                 .middle_gray = palette[MIDDLE_ORANGE],
                                 .dark_gray = palette[DARK_ORANGE]};
  // 32 bits/pixel = 4 B/pixel
  unsigned char *next_frame;
  pthread_t fb_update_thread, guitar_thread;
  VGAEmulator emulator;

  init_guitar_state(&controller_state);

  if (EMULATING_VGA) {
    printf("Running in VGA EMULATION MODE\n");
  }

  if ((framebuffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4)) == NULL) {
    perror("Error allocating framebuffer!\n");
    return 1;
  }

  SCREEN_LINE_LENGTH = WINDOW_WIDTH * 4;

  if ((next_frame = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4)) == NULL) {
    perror("Error allocating next_frame!\n");
    return 1;
  }

  // Set black background by default
  memset(framebuffer, 0, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
  // Load necessary sprites into memory
  sprite GH_circle_base = load_sprite("sprites/GH-Circle.png");
  // Generate the sprites for the notes
  generated_circles note_circles =
      generate_circles(GH_circle_base, green_colors, red_colors, yellow_colors,
                       blue_colors, orange_colors);
  // Generate the sprites for the indicators to play:
  green_colors.white = BACKGROUND_COLOR;
  red_colors.white = BACKGROUND_COLOR;
  yellow_colors.white = BACKGROUND_COLOR;
  blue_colors.white = BACKGROUND_COLOR;
  orange_colors.white = BACKGROUND_COLOR;
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

  // Set up VGA emulator. Requires libsdl2-dev
  if (EMULATING_VGA) {
    if (VGAEmulator_init(&emulator, framebuffer, &controller_state))
      return 1;
  } else {
    // Set up VGA framebuffer connection
    if ((vga_framebuffer_fd = open("/dev/vga_framebuffer", O_WRONLY)) == -1) {
      perror("could not open /dev/vga_framebuffer\n");
      return -1;
    }

    if ((guitar_fd = open("/dev/note_reader", O_RDONLY)) == -1) {
      perror("could not open /dev/note_reader\n");
      return -1;
    }

    if (pthread_create(&fb_update_thread, NULL, &update_framebuffer, NULL) !=
        0) {
      perror("pthread_create(fb_update_thread) failed\n");
      return 1;
    }

    if (pthread_create(&guitar_thread, NULL, read_and_buffer_input, NULL) !=
        0) {
      perror("pthread_create(fb_update_thread) failed\n");
      return 1;
    }
  }

  note_row song_rows[NUM_NOTE_ROWS];

  char line[9]; // Buffer to store each line (8 characters + null terminator)
  FILE *file = fopen("single_note_comaless.txt", "r");

  int i = 0;
  while (fgets(line, sizeof(line), file) != NULL) {
    // Remove the newline character if present
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    if (strlen(line) == 8) {
      // printf("note: %s\n", line);
      note_row note_row;
      set_note(&note_row, line);
      song_rows[i++] = note_row;
    }
  }

  fclose(file);

  int current_bottom_row_idx = 0, num_note_rows = 100;
  double current_bottom_row_Y = 0;
  int note_duration = round((60.0 / SONG_BPM) / NOTES_PER_MEASURE * 1000);

  // The Y coordinate of the middle of the guitar state line
  int guitar_state_line_Y = WINDOW_HEIGHT - 24;
  // How many pixels of "margin" (top and bottom) to apply to each note
  int note_row_veritcal_padding = 8;
  // THe total height including the 24x24 px sprite and the margin
  int note_height_px = 24 + 2 * note_row_veritcal_padding;
  // How many pixels each note row has to move down the screen in one ms
  double note_row_pixels_per_ms = (double)(note_height_px) / note_duration;

  printf("---SONG INFORMATION---\n");
  printf("BPM: %d\n", SONG_BPM);
  printf("Beat duration: %dms\n", note_duration);
  printf("Note row pixels/ms: %f\n", note_row_pixels_per_ms);

  // TODO: any start menu here

  long long song_start_time = current_time_in_ms();
  long long last_draw_time = song_start_time;

  while (1) {
    // Fresh start
    memset(next_frame, 0, WINDOW_WIDTH * WINDOW_HEIGHT * 4);

    long long time_delta = current_time_in_ms() - last_draw_time;
    last_draw_time = current_time_in_ms();

    for (int row_on_screen = 0;
         row_on_screen < WINDOW_HEIGHT / note_height_px + 1; row_on_screen++) {
      if (current_bottom_row_idx + row_on_screen >= num_note_rows)
        break; // We've run out of notes

      note_row row = song_rows[current_bottom_row_idx + row_on_screen];

      int row_y = round(current_bottom_row_Y - note_height_px * row_on_screen);

      if (row.green)
        draw_sprite(note_circles.green, next_frame, color_cols_x.green, row_y);
      if (row.red)
        draw_sprite(note_circles.red, next_frame, color_cols_x.red, row_y);
      if (row.yellow)
        draw_sprite(note_circles.yellow, next_frame, color_cols_x.yellow,
                    row_y);
      if (row.blue)
        draw_sprite(note_circles.blue, next_frame, color_cols_x.blue, row_y);
      if (row.orange)
        draw_sprite(note_circles.orange, next_frame, color_cols_x.orange,
                    row_y);
    }

    current_bottom_row_Y += note_row_pixels_per_ms * time_delta;

    pthread_mutex_lock(&controller_mutex);
    if (controller_state.strum) {
      // Is the bottom note in a playable range, and did we try?
      if (current_bottom_row_Y <= guitar_state_line_Y + 12 &&
          current_bottom_row_Y >= guitar_state_line_Y - 12) {
        if (hit_notes(controller_state, song_rows[current_bottom_row_idx])) {
          // We hit the note!
          current_bottom_row_idx++;
          current_bottom_row_Y -= (24 + 2 * note_row_veritcal_padding);
        } else
          printf("MISS\n");
      } else {
        printf("MISS (None to hit)\n");
      }
    }

    // Draw the Guitar state line
    draw_sprite(controller_state.green ? play_circles_held.green
                                       : play_circles_released.green,
                next_frame, color_cols_x.green, guitar_state_line_Y);
    draw_sprite(controller_state.red ? play_circles_held.red
                                     : play_circles_released.red,
                next_frame, color_cols_x.red, guitar_state_line_Y);
    draw_sprite(controller_state.yellow ? play_circles_held.yellow
                                        : play_circles_released.yellow,
                next_frame, color_cols_x.yellow, guitar_state_line_Y);
    draw_sprite(controller_state.blue ? play_circles_held.blue
                                      : play_circles_released.blue,
                next_frame, color_cols_x.blue, guitar_state_line_Y);
    draw_sprite(controller_state.orange ? play_circles_held.orange
                                        : play_circles_released.orange,
                next_frame, color_cols_x.orange, guitar_state_line_Y);
    pthread_mutex_unlock(&controller_mutex);

    // Is it time to shift the buffer because a note has gone off-screen?
    if (round(current_bottom_row_Y) >= WINDOW_HEIGHT + 8) {
      // The bottom row is off screen
      current_bottom_row_idx++;
      current_bottom_row_Y -= (24 + 2 * note_row_veritcal_padding);

      if (current_bottom_row_idx >= num_note_rows) {
        // We are done with the game
        break;
      }
    }

    // Push next frame to buffer
    pthread_mutex_lock(&framebuffer_mutex);
    memcpy(framebuffer, next_frame, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    pthread_mutex_unlock(&framebuffer_mutex);
  }

  // TODO: game end

  if (EMULATING_VGA)
    VGAEmulator_destroy(&emulator);
  // Clear sprites
  unload_sprite(GH_circle_base);
  unload_sprites(note_circles);
  unload_sprites(play_circles_released);
  unload_sprites(play_circles_held);

  if (EMULATING_VGA)
    free(framebuffer);

  return 0;
}
