#ifndef _VGA_FRAMEBUFFER_H
#define _VGA_FRAMEBUFFER_H

#include <linux/ioctl.h>
#ifdef __KERNEL__
#include <linux/io.h>
#else
#include <stdint.h>
#endif

typedef struct {
  uint32_t pixel_writedata;
} vga_framebuffer_arg_t;

#define VGA_FRAMEBUFFER_MAGIC 'q'

/* ioctls and their arguments */
#define VGA_FRAMEBUFFER_UPDATE                                                  \
  _IOW(VGA_FRAMEBUFFER_MAGIC, 1, vga_framebuffer_arg_t *)

#endif
