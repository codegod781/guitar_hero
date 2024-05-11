/* * Device driver for the VGA video generator
 *
 * A Platform device implemented using the misc subsystem
 *
 * Stephen A. Edwards
 * Columbia University
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * "make" to build
 * insmod vga_framebuffer.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree vga_framebuffer.c
 */

#include "vga_framebuffer.h"
#include "colors.h"
#include "global_consts.h"
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define DRIVER_NAME "vga_framebuffer"

/* Device registers */
#define FIRST_CHUNK(x) (x)

/*
 * Information about our device
 */
struct framebuffer_dev {
  struct resource res;    /* Resource: our registers */
  void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;

uint32_t pixel_writedata(unsigned char pixel_color, int pixel_row,
                         int pixel_col) {
  // Make sure pixel_color fits into 6 bits
  pixel_color &= 0x3F;

  // Make sure pixel_col fits into 8 bits
  pixel_col &= 0xFF;

  // Make sure pixel_row fits into 9 bits
  pixel_row &= 0x1FF;

  // Combine the values
  uint32_t result = 0;
  result |= (uint32_t)pixel_color;       // 6 least significant bits
  result |= ((uint32_t)pixel_col << 6);  // Next 8 bits
  result |= ((uint32_t)pixel_row << 14); // Next 9 bits

  return result;
}

/*
 * Write segments of a single digit
 * Assumes digit is in range and the device information has been set up
 */
static void write_background(unsigned char *framebuffer) {
  int pixel_row, pixel_col;
  uint32_t writedata;
  for (pixel_row = 0; pixel_row < WINDOW_HEIGHT; pixel_row++) {
    for (pixel_col = 0; pixel_col < WINDOW_WIDTH; pixel_col++) {
      unsigned char *pixel =
          framebuffer + (pixel_row * WINDOW_WIDTH + pixel_col) * 4;

      RGB pixel_rgb = {pixel[2], pixel[1], pixel[0]};

      writedata =
          pixel_writedata(get_color_from_rgb(pixel_rgb), pixel_row, pixel_col);
      iowrite32(writedata, FIRST_CHUNK(dev.virtbase));
    }
  }
}

/*
 * Handle ioctl() calls from userspace:
 * Read or write the segments on single digits.
 * Note extensive error checking of arguments
 */
static long vga_framebuffer_ioctl(struct file *f, unsigned int cmd,
                                  unsigned long arg) {
  vga_framebuffer_arg_t vfba;

  switch (cmd) {
  case VGA_FRAMEBUFFER_UPATE:
    if (copy_from_user(&vfba, (vga_framebuffer_arg_t *)arg,
                       sizeof(vga_framebuffer_arg_t)))
      return -EACCES;
    write_background(vfba.framebuffer);
    break;

  default:
    return -EINVAL;
  }

  return 0;
}

/* The operations our device knows how to do */
static const struct file_operations vga_framebuffer_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = vga_framebuffer_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice vga_framebuffer_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DRIVER_NAME,
    .fops = &vga_framebuffer_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init vga_framebuffer_probe(struct platform_device *pdev) {
  int ret;

  /* Register ourselves as a misc device: creates /dev/vga_framebuffer */
  ret = misc_register(&vga_framebuffer_misc_device);

  /* Get the address of our registers from the device tree */
  ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
  if (ret) {
    ret = -ENOENT;
    goto out_deregister;
  }

  /* Make sure we can use these registers */
  if (request_mem_region(dev.res.start, resource_size(&dev.res), DRIVER_NAME) ==
      NULL) {
    ret = -EBUSY;
    goto out_deregister;
  }

  /* Arrange access to our registers */
  dev.virtbase = of_iomap(pdev->dev.of_node, 0);
  if (dev.virtbase == NULL) {
    ret = -ENOMEM;
    goto out_release_mem_region;
  }

  return 0;

out_release_mem_region:
  release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
  misc_deregister(&vga_framebuffer_misc_device);
  return ret;
}

/* Clean-up code: release resources */
static int vga_framebuffer_remove(struct platform_device *pdev) {
  iounmap(dev.virtbase);
  release_mem_region(dev.res.start, resource_size(&dev.res));
  misc_deregister(&vga_framebuffer_misc_device);
  return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id vga_framebuffer_of_match[] = {
    {.compatible = "csee4840,vga_framebuffer-1.0"},
    {},
};
MODULE_DEVICE_TABLE(of, vga_framebuffer_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver vga_framebuffer_driver = {
    .driver =
        {
            .name = DRIVER_NAME,
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(vga_framebuffer_of_match),
        },
    .remove = __exit_p(vga_framebuffer_remove),
};

/* Called when the module is loaded: set things up */
static int __init vga_framebuffer_init(void) {
  pr_info(DRIVER_NAME ": init\n");
  return platform_driver_probe(&vga_framebuffer_driver, vga_framebuffer_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit vga_framebuffer_exit(void) {
  platform_driver_unregister(&vga_framebuffer_driver);
  pr_info(DRIVER_NAME ": exit\n");
}

module_init(vga_framebuffer_init);
module_exit(vga_framebuffer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dan Ivanovich & Patrick Cronin, Columbia University");
MODULE_DESCRIPTION("VGA Framebuffer Driver");
