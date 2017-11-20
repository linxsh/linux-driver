/*
 *  HDMI Platform Device Driver
 *
 * Copyright (C) 1991-2017 linxsh, Ltd
 * All rights reserved!
 *
 * driver.c: HDMI Platform Driver Implement
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include "ep952_core.h"
#include "log.h"

#define HDMI_MODULE_NAME "hdmi,ep952"
#define HDMI_CLASS_NAME  "hdmi:ep952"
#define HDMI_DEVICE_NAME "hdmi-ep952"

static int hdmi_major = 0;
static int hdmi_minor = 0;

typedef struct {
    struct class   *class;
	struct mutex    mutex;
	struct cdev     cdev;
	int             irq;
    void __iomem   *regs;
} DEV_PRIV;

static DEV_PRIV s_dev_data;

static int hdmi_open(struct inode *inode, struct file *filp)
{
	void *handle = (void*)ep952CoreOpen();

	if (handle) {
		filp->private_data = handle;
		return 0;
	}

	return -1;
}

static int hdmi_release(struct inode *inode, struct file *filp)
{
	if (filp->private_data)
		return ep952CoreClose(filp->private_data);

	return -1;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
static long hdmi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int  hdmi_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	if (filp->private_data)
		return ep952CoreIoctl(filp->private_data, cmd, (void*)arg);

	return -1;
}


static struct file_operations hdmi_fops =
{
	.owner          = THIS_MODULE,
	.open           = hdmi_open,
	.release        = hdmi_release,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
	.unlocked_ioctl = hdmi_ioctl,
#else
	.ioctl          = hdmi_ioctl,
#endif
};

static int __init device_driver_init(void)
{
	int result = -1;
	dev_t dev;

	if (hdmi_major) {
		dev = MKDEV(hdmi_major, hdmi_minor);
		result = register_chrdev_region(dev, 1, HDMI_MODULE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, hdmi_minor, 1, HDMI_MODULE_NAME);
		hdmi_major = MAJOR(dev);
	}

	if (result < 0) {
		LogFormat(ERROR, "%s:%d\n", __FILE__, __LINE__);
		return result;
	}

	s_dev_data.class = class_create(THIS_MODULE, HDMI_CLASS_NAME);
	if (IS_ERR(s_dev_data.class)) {
		LogFormat(ERROR, "%s:%d\n", __FILE__, __LINE__);
		return PTR_ERR(s_dev_data.class);
	}

	cdev_init(&s_dev_data.cdev, &hdmi_fops);
	s_dev_data.cdev.owner = THIS_MODULE;
	s_dev_data.cdev.ops = &hdmi_fops;

	result = cdev_add(&s_dev_data.cdev, dev, 1);
	if (result) {
		LogFormat(ERROR, "%s:%d\n", __FILE__, __LINE__);
		return result;
	}

	device_create(s_dev_data.class, NULL, dev, NULL, HDMI_DEVICE_NAME);

	result = ep952CoreInit();
	if (result) {
		LogFormat(ERROR, "%s:%d\n", __FILE__, __LINE__);
		return result;
	}

	return 0;
}

static void __exit device_driver_exit(void)
{
	ep952CoreDestory();

	unregister_chrdev_region(MKDEV(hdmi_major, hdmi_minor), 1);

	device_destroy(s_dev_data.class, MKDEV(hdmi_major, hdmi_minor));

	cdev_del(&s_dev_data.cdev);

	class_destroy(s_dev_data.class);
}

module_init(device_driver_init);
module_exit(device_driver_exit);

MODULE_AUTHOR("linxsh");
MODULE_DESCRIPTION("hdmi for ep952");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hdmi-ep952");
