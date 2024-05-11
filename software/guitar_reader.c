/* Device driver for the Guitar Hero Guitar
 *
 * Patrick Cronin, Dan Ivanovich, & Kiryl Beliauski
 * Columbia University CSEE 4840 - Embedded Systems
 *
 * Adapted from code by Stephen A. Edwards, Columbia University
 * A Platform device implemented using the misc subsystem
 *
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * Check code style with
 * checkpatch.pl --file --no-tree guitar_reader.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "guitar_reader.h"

#define DRIVER_NAME "note_reader"

/* Device registers */
#define FIRST_CHUNK(x) (x)

/*
 * Information about our device
 */
struct guitar_reader_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;


/*
* Reads the guitar state
*/
int read_guitar_state(void)
{
	return ioread8(dev.virtbase);
}

/*
 * Handle ioctl() calls from userspace:
 * Read or write the segments on single digits.
 * Note extensive error checking of arguments
 */
static long guitar_reader_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	// int chunk = write_background();
	int chunk = read_guitar_state();
	

	switch (cmd) {

	case GUITAR_READER_READ:
		if (copy_to_user((void __user *)arg, &chunk, sizeof(int)))
			return -EACCES;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations guitar_reader_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = guitar_reader_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice guitar_reader_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &guitar_reader_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init guitar_reader_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/guitar_reader */
	ret = misc_register(&guitar_reader_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
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
	misc_deregister(&guitar_reader_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int guitar_reader_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&guitar_reader_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id guitar_reader_of_match[] = {
	{ .compatible = "csee4840,note_reader-1.0" },
	{},
};
MODULE_DEVICE_TABLE(of, guitar_reader_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver guitar_reader_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(guitar_reader_of_match),
	},
	.remove	= __exit_p(guitar_reader_remove),
};

/* Called when the module is loaded: set things up */
static int __init guitar_reader_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&guitar_reader_driver, guitar_reader_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit guitar_reader_exit(void)
{
	platform_driver_unregister(&guitar_reader_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(guitar_reader_init);
module_exit(guitar_reader_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dan Ivanovich & Patrick Cronin, Columbia University");
MODULE_DESCRIPTION("VGA Framebuffer Driver");
