#ifndef _VGA_H
#  define _VGA_H

#define FBOPEN_DEV -1          /* Couldn't open the device */
#define FBOPEN_FSCREENINFO -2  /* Couldn't read the fixed info */
#define FBOPEN_VSCREENINFO -3  /* Couldn't read the variable info */
#define FBOPEN_MMAP -4         /* Couldn't mmap the framebuffer memory */
#define FBOPEN_BPP -5          /* Unexpected bits-per-pixel */

// Contains everything lab2.c needs to know about the screen to avoid more
// hard-coded magic numbers, eg the numbers of rows of text per screen
typedef struct {
  struct fb_var_screeninfo *fb_vinfo;
  struct fb_fix_screeninfo *fb_finfo;
} screen_info;

unsigned char * fbopen(void);
extern screen_info get_fb_screen_info(void);
#endif
