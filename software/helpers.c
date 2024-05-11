#include "helpers.h"
#include <sys/time.h>
#include <stddef.h>

long long current_time_in_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long long ms =
      (tv.tv_sec * 1000LL) +
      (tv.tv_usec /
       1000); // Convert seconds to ms and add microseconds converted to ms
  return ms;
}

/* Constructs a properly-formatted writedata packet for the Avalon Bus */
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