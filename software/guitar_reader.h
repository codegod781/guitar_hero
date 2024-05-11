#ifndef _GUITAR_READER_H
#define _GUITAR_READER_H

#include <linux/ioctl.h>

#define GUITAR_READER_MAGIC 'q'

/* ioctls and their arguments */
#define GUITAR_READER_READ  _IOR(GUITAR_READER_MAGIC, 1, int *)

#endif
