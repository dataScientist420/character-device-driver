/******************************************************************************
                          ____     ____                                  
                 		 / __/	  / __/__  __ _____________                            
                		/ /_	 _\ \/ _ \/ // / __/ __/ -_|                           
               			\__/	/___/\___/\___/_/  \__/\__/                            
                                                                                                                                                                          
*******************************************************************************
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
 @file: charmodule.c
 @author: Victor Neville
 @description: Implementation of a simple character device driver. 	  			  
 @date: 13-05-2017
 *****************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

/* CONSTANTS */
#define BUF_LENGTH  1024
#define NAME        "s33drax"
#define ERROR       -1

/******************************************************************************
 						Declaration of functions
******************************************************************************/
static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *); 
static void free_kalloc(void);

/* Global structures */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};

static struct buffer_device {
	char *buf;
	struct cdev *cdev;
	struct class *class;
	struct semaphore sem;
	dev_t num;	
} bdev;

/******************************************************************************
 Initializes the driver when command "insmod chardriver.ko" is executed.
******************************************************************************/
static int __init mod_init(void) 
{ 	
  	bdev.buf = NULL;
	bdev.cdev = NULL;
	bdev.class = NULL;	  

	// allocates a range of character device numbers 	
	if (alloc_chrdev_region(&bdev.num, 0, 1, NAME) < 0) {
		printk(KERN_ALERT "\n%s: failed to allocate a major number", NAME);
		return ERROR;
	}
	// allocate a device class
	if ((bdev.class = class_create(THIS_MODULE, NAME)) == NULL) {
		printk(KERN_ALERT "\n%s: failed to allocate class", NAME);
		free_kalloc();
		return ERROR;
	}
	// allocate a device file
	if (device_create(bdev.class, NULL, bdev.num, NULL, NAME) == NULL) {
		printk(KERN_ALERT "\n%s: failed to allocate device file", NAME);
		free_kalloc();
		return ERROR;
	}	
	// allocates a character device structure 
	bdev.cdev = cdev_alloc();
	bdev.cdev->ops = &fops;
	bdev.cdev->owner = THIS_MODULE;

	// allocates a buffer of size BUF_LENGTH
	if ((bdev.buf = kcalloc(BUF_LENGTH, sizeof(char), GFP_KERNEL)) == NULL) {
		printk(KERN_ALERT "\n%s: failed to allocate buffer", NAME);
		free_kalloc();
		return ERROR;
	}
	// add device to the kernel 
	if (cdev_add(bdev.cdev, bdev.num, 1) == ERROR) {
		printk(KERN_ALERT "\n%s: unable to add char device", NAME);
		free_kalloc();
		return ERROR;
	}
	// initialize semaphore
	sema_init(&bdev.sem, 1);

	printk(KERN_ALERT "\n%s: loaded module", NAME);
	return 0;
}

/******************************************************************************
 Exits the driver when command "rmmod chardriver" is executed.
******************************************************************************/
static void __exit mod_exit(void) 
{
  	free_kalloc();
	printk(KERN_ALERT "\n%s: unloaded module", NAME); 
}

/******************************************************************************
 Opens the character device.
******************************************************************************/
static int device_open(struct inode *inode, struct file *file)
{ 
	printk(KERN_ALERT "\n%s: opened device", NAME);	
	return 0;
}

/******************************************************************************
 Closes the character device.
******************************************************************************/
static int device_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "\n%s: device close", NAME);	
	return 0;
}

/******************************************************************************
 Called when user wants to send information to the device and copies data 
 from kernel space to user space.
******************************************************************************/
static ssize_t device_read(struct file *file, char *dst, size_t count, 
							loff_t *f_offset) {
	
	printk(KERN_INFO "\n%s: reading from device", NAME);
	return copy_to_user(dst, bdev.buf, count);
}

/******************************************************************************
 Called when user wants to send information to the device and copies data 
 from user space to kernel space.
******************************************************************************/
static ssize_t device_write(struct file *file, const char *src, size_t count, 
							loff_t *f_offset) {
	
		
	// return an error code when device is already open for writting	
	if (down_interruptible(&bdev.sem))
		return -ERESTARTSYS;

	printk(KERN_INFO "\n%s: writing to device", NAME);
	
	if (copy_from_user(bdev.buf, src, count) != 0)
		return -EFAULT;

	// free semaphore
	up(&bdev.sem);
	return 0;
}

/******************************************************************************
 Frees the dynamic memory allocations.
******************************************************************************/
void free_kalloc()
{
	// if pointers have valid addresses, the allocations are freed	
	if (bdev.buf)
		kfree(bdev.buf);	
	if (bdev.cdev)
		cdev_del(bdev.cdev);
	if (bdev.class && bdev.num) {
		device_destroy(bdev.class, bdev.num);
		class_destroy(bdev.class);
		unregister_chrdev_region(bdev.num, 1);
	}
}

/******************************************************************************
 Assign module: information, entry point and exit point.
******************************************************************************/
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Victor Neville");
MODULE_DESCRIPTION("Test Driver Module");
module_init(mod_init);
module_exit(mod_exit);

/*****************************************************************************/
