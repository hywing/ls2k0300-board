#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define GPIOLED_CNT			1		  
#define GPIOLED_NAME		"led"	
#define LEDOFF 				0		
#define LEDON 				1		
#define GPIO_LED 			83

struct gpioled_dev{
	dev_t devid;		
	struct cdev cdev;	
	struct class *class;	
	struct device *device;	
	int major;			
	int minor;			
	struct device_node	*nd; 
	int led;		
};

struct gpioled_dev gpioled;	

static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &gpioled; 
	return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;
	struct gpioled_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];	

	if(ledstat == LEDON) {	
		gpio_set_value(dev->led, 0);	
	} else if(ledstat == LEDOFF) {
		gpio_set_value(dev->led, 1);	
	}
	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations gpioled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

static int __init led_init(void)
{
	int ret = 0;

	gpioled.led = GPIO_LED;
	if(gpioled.led < 0) {
		printk("can't get led-gpio");
		return -EINVAL;
	}

	ret = gpio_request(gpioled.led, "LED-GPIO");
    if (ret) {
        printk(KERN_ERR "gpioled: Failed to request led-gpio\n");
        return ret;
	}

	ret = gpio_direction_output(gpioled.led, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

	if (gpioled.major) {		
		gpioled.devid = MKDEV(gpioled.major, 0);
		ret = register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
		if(ret < 0) {
			pr_err("cannot register %s char driver [ret=%d]\n", GPIOLED_NAME, GPIOLED_CNT);
			goto free_gpio;
		}
	} else {					
		ret = alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
		if(ret < 0) {
			pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", GPIOLED_NAME, ret);
			goto free_gpio;
		}
		gpioled.major = MAJOR(gpioled.devid);	
		gpioled.minor = MINOR(gpioled.devid);	
	}
	
	gpioled.cdev.owner = THIS_MODULE;
	cdev_init(&gpioled.cdev, &gpioled_fops);
	cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);
	if(ret < 0)
		goto del_unregister;
		
	gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
	if (IS_ERR(gpioled.class)) {
		goto del_cdev;
	}

	gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
	if (IS_ERR(gpioled.device)) {
		goto destroy_class;
	}
	return 0;
	
destroy_class:
	class_destroy(gpioled.class);
del_cdev:
	cdev_del(&gpioled.cdev);
del_unregister:
	unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
free_gpio:
	gpio_free(gpioled.led);
	return -EIO;
}

static void __exit led_exit(void)
{
	cdev_del(&gpioled.cdev);
	unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); 
	device_destroy(gpioled.class, gpioled.devid);
	class_destroy(gpioled.class);
	gpio_free(gpioled.led); 
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");