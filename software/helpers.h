#ifndef HELPERS_H
#define HELPERS_H

#ifdef __KERNEL__
#include <linux/io.h>
#else
#include <stdint.h>
#endif

long long current_time_in_ms();
uint32_t pixel_writedata(unsigned char pixel_color, int pixel_row,
                         int pixel_col);

#endif /* HELPERS_H */
