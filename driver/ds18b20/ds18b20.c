#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/errno.h>

#define DS18B20_DEV_NUM			1		  
#define DS18B20_DEV		        "ds18b20"	
#define DS18B20_GPIO            48
#define DS18B20_DQ_OUT(x)       gpio_direction_output(ds18b20.gpio, x)
#define DS18B20_DQ_IN           ds18b20_get_io()

struct ds18b20_dev {
    dev_t dev_id;		
	struct cdev cdev;	
	struct class *class;	
	struct device *device;	
	int major;			
	int minor;	
    int gpio;				
    unsigned char data[2]; 
	struct timer_list timer;	
    struct work_struct work;
};

struct ds18b20_dev ds18b20;

static int ds18b20_get_io(void)
{
    gpio_direction_input(ds18b20.gpio);
    return gpio_get_value(ds18b20.gpio); 
}

static void ds18b20_reset(void)
{
    DS18B20_DQ_OUT(0);  /* 拉低DQ,复位 */
    udelay(750);      /* 拉低750us */
    DS18B20_DQ_OUT(1);  /* DQ=1, 释放复位 */
    udelay(15);       /* 延迟15US */
}

uint8_t ds18b20_check(void)
{
    uint8_t retry = 0;
    uint8_t rval = 0;

    while (DS18B20_DQ_IN && retry < 200)    /* 等待DQ变低, 等待200us */
    {
        retry++;
        udelay(1);
    }

    if (retry >= 200)
    {
        rval = 1;
    }
    else
    {
        retry = 0;

        while (!DS18B20_DQ_IN && retry < 240)   /* 等待DQ变高, 等待240us */
        {
            retry++;
            udelay(1);
        }

        if (retry >= 240) rval = 1;
    }

    return rval;
}

static uint8_t ds18b20_read_bit(void)
{
    uint8_t data = 0;
    DS18B20_DQ_OUT(0);
    udelay(2);
    DS18B20_DQ_OUT(1);
    udelay(12);

    if (DS18B20_DQ_IN)
    {
        data = 1;
    }

    udelay(50);
    return data;
}

static uint8_t ds18b20_read_byte(void)
{
    uint8_t i, b, data = 0;

    for (i = 0; i < 8; i++)
    {
        b = ds18b20_read_bit(); /* DS18B20先输出低位数据 ,高位数据后输出 */

        data |= b << i;         /* 填充data的每一位 */
    }

    return data;
}

static void ds18b20_write_byte(uint8_t data)
{
    uint8_t j;
    for (j = 1; j <= 8; j++)
    {
        if (data & 0x01)
        {
            DS18B20_DQ_OUT(0);  /*  Write 1 */
            udelay(2);
            DS18B20_DQ_OUT(1);
            udelay(60);
        }
        else
        {
            DS18B20_DQ_OUT(0);  /*  Write 0 */
            udelay(60);
            DS18B20_DQ_OUT(1);
            udelay(2);
        }

        data >>= 1;             /* 右移,获取高一位数据 */
    }
}

static void ds18b20_start(void)
{
    ds18b20_reset();
    ds18b20_check();
    ds18b20_write_byte(0xcc);   /*  skip rom */
    ds18b20_write_byte(0x44);   /*  convert */
}

static int ds18b20_init(void)
{
    gpio_direction_output(ds18b20.gpio, 0);
    ds18b20_reset();
    return ds18b20_check();	
}
 
static int ds18b20_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t ds18b20_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt) 
{
    int ret;
    ret = copy_to_user(buf, &ds18b20.data[0], 2);
	if(ret)
        return -ENOMEM;
	return ret;
}

static struct file_operations ds18b20_fops = {
	.owner	= THIS_MODULE,
	.open = ds18b20_open,
	.read	= ds18b20_read,
};

static void ds18b20_work_callback(struct work_struct *work)
{
    ds18b20_start();                            /*  ds1820 start convert */
    ds18b20_reset();
    ds18b20_check();
    ds18b20_write_byte(0xcc);                   /*  skip rom */
    ds18b20_write_byte(0xbe);                   /*  convert */
    ds18b20.data[0] = ds18b20_read_byte();      /*  LSB */
    ds18b20.data[1] = ds18b20_read_byte();      /*  MSB */
}

static void ds18b20_timer_callback(struct timer_list *arg)
{
    schedule_work(&ds18b20.work);	
    mod_timer(&ds18b20.timer, jiffies + (1000 * HZ/1000));	
}

static int ds18b20_module_init(void)
{
    int ret = 0;
    ds18b20.gpio = DS18B20_GPIO;
    if (!gpio_is_valid(ds18b20.gpio)) {
        return -EINVAL;
    }

    ret = gpio_request(ds18b20.gpio, "DS18B20-GPIO");
    if (ret) {
        printk(KERN_ERR "ds18b20 : Failed to request gpio\n");
        return ret;
	}

    ds18b20_init();

    if (ds18b20.major) {		
		ds18b20.dev_id = MKDEV(ds18b20.major, 0);
		ret = register_chrdev_region(ds18b20.dev_id, DS18B20_DEV_NUM, DS18B20_DEV);
		if(ret < 0) {
			pr_err("cannot register %s char driver [ret=%d]\n", DS18B20_DEV, DS18B20_DEV_NUM);
			goto free_gpio;
		}
	} 
    else {					
		ret = alloc_chrdev_region(&ds18b20.dev_id, 0, DS18B20_DEV_NUM, DS18B20_DEV);
		if(ret < 0) {
			pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", DS18B20_DEV, ret);
			goto free_gpio;
		}
		ds18b20.major = MAJOR(ds18b20.dev_id);	
		ds18b20.minor = MINOR(ds18b20.dev_id);	
	}
	
	ds18b20.cdev.owner = THIS_MODULE;
	cdev_init(&ds18b20.cdev, &ds18b20_fops);
	cdev_add(&ds18b20.cdev, ds18b20.dev_id, DS18B20_DEV_NUM);
	if(ret < 0)
		goto del_unregister;
		
	ds18b20.class = class_create(THIS_MODULE, DS18B20_DEV);
	if (IS_ERR(ds18b20.class)) {
		goto del_cdev;
	}

	ds18b20.device = device_create(ds18b20.class, NULL, ds18b20.dev_id, NULL, DS18B20_DEV);
	if (IS_ERR(ds18b20.device)) {
		goto destroy_class;
	}

	timer_setup(&ds18b20.timer, ds18b20_timer_callback, 0);
    ds18b20.timer.expires=jiffies + msecs_to_jiffies(1000);
    add_timer(&ds18b20.timer);
	INIT_WORK(&ds18b20.work, ds18b20_work_callback);

    return 0;

destroy_class:
	class_destroy(ds18b20.class);
del_cdev:
	cdev_del(&ds18b20.cdev);
del_unregister:
	unregister_chrdev_region(ds18b20.dev_id, DS18B20_DEV_NUM);
free_gpio:
	gpio_free(ds18b20.gpio);
	return -EIO;
}


static void ds18b20_module_exit(void)
{
	cdev_del(&ds18b20.cdev);
	unregister_chrdev_region(ds18b20.dev_id, DS18B20_DEV_NUM); 
	device_destroy(ds18b20.class, ds18b20.dev_id);
	class_destroy(ds18b20.class);
	del_timer(&ds18b20.timer);
    cancel_work_sync(&ds18b20.work);
    gpio_free(ds18b20.gpio); 
}

module_init(ds18b20_module_init);
module_exit(ds18b20_module_exit);
MODULE_LICENSE("GPL");