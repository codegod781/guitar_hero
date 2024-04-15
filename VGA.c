/*
 * VGA.c: VGA communicator
 *
 * Assumes 32bpp
 *
 * References: Lab 2
 */

#include "VGA.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <linux/fb.h>
#include <stdio.h>

#define FBDEV "/dev/fb0"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define BITS_PER_PIXEL 32

struct fb_var_screeninfo fb_vinfo;
struct fb_fix_screeninfo fb_finfo;
unsigned char *framebuffer;

/*
 * Open the framebuffer to prepare it to be written to.  Returns 0 on success
 * or one of the FBOPEN_... return codes if something went wrong.
 */
unsigned char * fbopen()
{
  int fd = open(FBDEV, O_RDWR); /* Open the device */
  if (fd == -1) return NULL;

  if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_finfo)) /* Get fixed info about fb */
    return NULL;

  if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_vinfo)) /* Get varying info about fb */
    return NULL;

  if (fb_vinfo.bits_per_pixel != BITS_PER_PIXEL) return NULL; /* Unexpected */

  framebuffer = mmap(0, fb_finfo.smem_len, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, 0);
  if (framebuffer == (unsigned char *)-1) return NULL;

  return framebuffer;
}

// This should ONLY be called after fbopen()!
screen_info get_fb_screen_info() {
  if (framebuffer == NULL) {
    fprintf(stderr, "Warning: get_fb_screen_info() called before fbopen()\n");
  }

  screen_info info;
  info.fb_vinfo = &fb_vinfo;
  info.fb_finfo = &fb_finfo;

  return info;
}