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
#define OFF 				0		
#define ON 					1		
#define GPIO_LED 			83

struct my_led_dev{
	dev_t dev_id;		
	struct cdev cdev;	
	struct class *class;	
	struct device *device;	
	int major;			
	int minor;			
	struct device_node	*nd; 
	int led;		
};

struct my_led_dev led_dev;	

static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &led_dev; 
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
	struct my_led_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];	

	if(ledstat == ON) {	
		gpio_set_value(dev->led, 0);	
	} else if(ledstat == OFF) {
		gpio_set_value(dev->led, 1);	
	}
	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations led_dev_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

static int __init led_init(void)
{
	int ret = 0;

	led_dev.led = GPIO_LED;
	if(led_dev.led < 0) {
		printk("can't get led-gpio");
		return -EINVAL;
	}

	ret = gpio_request(led_dev.led, "LED-GPIO");
    if (ret) {
        printk(KERN_ERR "led_dev: Failed to request led-gpio\n");
        return ret;
	}

	ret = gpio_direction_output(led_dev.led, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

	if (led_dev.major) {		
		led_dev.dev_id = MKDEV(led_dev.major, 0);
		ret = register_chrdev_region(led_dev.dev_id, GPIOLED_CNT, GPIOLED_NAME);
		if(ret < 0) {
			pr_err("cannot register %s char driver [ret=%d]\n", GPIOLED_NAME, GPIOLED_CNT);
			goto free_gpio;
		}
	} else {					
		ret = alloc_chrdev_region(&led_dev.dev_id, 0, GPIOLED_CNT, GPIOLED_NAME);
		if(ret < 0) {
			pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", GPIOLED_NAME, ret);
			goto free_gpio;
		}
		led_dev.major = MAJOR(led_dev.dev_id);	
		led_dev.minor = MINOR(led_dev.dev_id);	
	}
	
	led_dev.cdev.owner = THIS_MODULE;
	cdev_init(&led_dev.cdev, &led_dev_fops);
	cdev_add(&led_dev.cdev, led_dev.dev_id, GPIOLED_CNT);
	if(ret < 0)
		goto del_unregister;
		
	led_dev.class = class_create(THIS_MODULE, GPIOLED_NAME);
	if (IS_ERR(led_dev.class)) {
		goto del_cdev;
	}

	led_dev.device = device_create(led_dev.class, NULL, led_dev.dev_id, NULL, GPIOLED_NAME);
	if (IS_ERR(led_dev.device)) {
		goto destroy_class;
	}
	return 0;
	
destroy_class:
	class_destroy(led_dev.class);
del_cdev:
	cdev_del(&led_dev.cdev);
del_unregister:
	unregister_chrdev_region(led_dev.dev_id, GPIOLED_CNT);
free_gpio:
	gpio_free(led_dev.led);
	return -EIO;
}

static void __exit led_exit(void)
{
	cdev_del(&led_dev.cdev);
	unregister_chrdev_region(led_dev.dev_id, GPIOLED_CNT); 
	device_destroy(led_dev.class, led_dev.dev_id);
	class_destroy(led_dev.class);
	gpio_free(led_dev.led); 
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");