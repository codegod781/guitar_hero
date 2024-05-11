#ifndef _VGA_BALL_H
#define _VGA_BALL_H

#include <linux/ioctl.h>

typedef struct {
  unsigned char *framebuffer;
} vga_framebuffer_arg_t;

#define VGA_FRAMEBUFFER_MAGIC 'q'

/* ioctls and their arguments */
#define VGA_FRAMEBUFFER_UPATE                                                  \
  _IOW(VGA_FRAMEBUFFER_MAGIC, 1, vga_framebuffer_arg_t *)

#endif
