#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/irq.h>
 
#define DEVICE_CNT		1
#define DEVICE_NAME		"aht20"
#define DEV_ID_NAME 	"loongson,aht20"

struct aht20_dev {
	struct i2c_client *client;	
	dev_t dev_id;			
	struct cdev cdev;	
	struct class *class;	
	struct device *device;
};

static struct i2c_client *my_i2c_client;

static int aht20_read_regs(struct aht20_dev *dev, u8 reg, void *val, int len)
{
	int ret;
	struct i2c_msg msg[2];
	struct i2c_client *client = (struct i2c_client *)dev->client;

	msg[0].addr = client->addr;		
	msg[0].flags = 0;				
	msg[0].buf = &reg;			
	msg[0].len = 1;				

	msg[1].addr = client->addr;		
	msg[1].flags = I2C_M_RD;			
	msg[1].buf = val;			
	msg[1].len = len;		

	ret = i2c_transfer(client->adapter, msg, 2);
	if(ret == 2) {
		ret = 0;
	} else {
		printk("i2c rd failed=%d reg=%06x len=%d\n",ret, reg, len);
		ret = -EREMOTEIO;
	}
	return ret;
}

static s32 aht20_write_regs(struct aht20_dev *dev, u8 reg, u8 *buf, u8 len)
{
	u8 b[256];
	struct i2c_msg msg;
	struct i2c_client *client = (struct i2c_client *)dev->client;
	
	b[0] = reg;				
	memcpy(&b[1],buf,len);		
		
	msg.addr = client->addr;	
	msg.flags = 0;			

	msg.buf = b;	
	msg.len = len + 1;		

	return i2c_transfer(client->adapter, &msg, 1);
}

static unsigned char aht20_read_reg(struct aht20_dev *dev, u8 reg)
{
	u8 data = 0;

	aht20_read_regs(dev, reg, &data, 1);
	return data;
}

void ATH20_Read_CTdata(struct aht20_dev *dev, uint32_t *ct)
{
    uint32_t RetuData = 0;
	uint16_t cnt = 0;
    uint8_t Data[10];
    uint8_t tmp[10];
	uint8_t val = 0;

    tmp[0] = 0x33;
    tmp[1] = 0x00;

    aht20_write_regs(dev, 0xAC, tmp, 2);

	mdelay(75);//等待75ms

	while((((val = aht20_read_reg(dev, 0x00))&0x80) == 0x80))
	{
        mdelay(1);
        if(cnt++ >= 100)
        {
            break;
        }
	}

    aht20_read_regs(dev, 0x00, Data, 7);

	RetuData = 0;
    RetuData = (RetuData|Data[1]) << 8;
	RetuData = (RetuData|Data[2]) << 8;
	RetuData = (RetuData|Data[3]);
	RetuData = RetuData >> 4;
	ct[0] = RetuData;

    RetuData = 0;
	RetuData = (RetuData|Data[3]) << 8;
	RetuData = (RetuData|Data[4]) << 8;
	RetuData = (RetuData|Data[5]);
	RetuData = RetuData&0xfffff;
	ct[1] = RetuData;
}

void aht20_readdata(struct aht20_dev *dev, uint32_t *CT_data)
{
	ATH20_Read_CTdata(dev, CT_data);
}


uint8_t ATH20_Read_Cal_Enable(struct aht20_dev *dev)
{
    uint8_t val = aht20_read_reg(dev, 0x00);
    if((val & 0x68) == 0x08) 
        return 1;
    else
        return 0;
}

static int aht20_open(struct inode *inode, struct file *filp)
{
    uint8_t count;
	struct cdev *cdev = filp->f_path.dentry->d_inode->i_cdev;
	struct aht20_dev *aht20 = container_of(cdev, struct aht20_dev, cdev);

	uint8_t tmp[10];

    mdelay(40);

    tmp[0] = 0x08;
    tmp[1] = 0x00;

    aht20_write_regs(aht20, 0xBE, tmp, 2);

    mdelay(500);
    count = 0;

    while(ATH20_Read_Cal_Enable(aht20) == 0)
    {
        aht20_write_regs(aht20, 0xBA, tmp, 0);
        mdelay(200);

        aht20_write_regs(aht20, 0xBE, tmp, 2);

        count++;
        if(count >= 10)
            return 0;
        mdelay(500);
    }

	return 0;
}

static ssize_t aht20_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	uint32_t data[2];
	long err = 0;

	struct cdev *cdev = filp->f_path.dentry->d_inode->i_cdev;
	struct aht20_dev *dev = container_of(cdev, struct aht20_dev, cdev);
	
	aht20_readdata(dev, data);

	err = copy_to_user(buf, data, sizeof(data));
	return err;
}

static int aht20_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations aht20_ops = {
	.owner = THIS_MODULE,
	.open = aht20_open,
	.read = aht20_read,
	.release = aht20_release,
};
 
static const struct i2c_device_id aht20_dev_id[] = {
	{ DEV_ID_NAME, 0 },
	{ }
};
 
static int i2c_drv_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct aht20_dev *aht20;

    my_i2c_client = client;    
 
	aht20 = devm_kzalloc(&client->dev, sizeof(*aht20), GFP_KERNEL);
	if(!aht20)
		return -ENOMEM;
		
	ret = alloc_chrdev_region(&aht20->dev_id, 0, DEVICE_CNT, DEVICE_NAME);
	if(ret < 0) {
		pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", DEVICE_NAME, ret);
		return -ENOMEM;
	}

	aht20->cdev.owner = THIS_MODULE;
	cdev_init(&aht20->cdev, &aht20_ops);
	
	ret = cdev_add(&aht20->cdev, aht20->dev_id, DEVICE_CNT);
	if(ret < 0) {
		goto del_unregister;
	}
	
	aht20->class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(aht20->class)) {
		goto del_cdev;
	}

	aht20->device = device_create(aht20->class, NULL, aht20->dev_id, NULL, DEVICE_NAME);
	if (IS_ERR(aht20->device)) {
		goto destroy_class;
	}

	aht20->client = client;
	i2c_set_clientdata(client,aht20);

	return 0;

destroy_class:
	device_destroy(aht20->class, aht20->dev_id);
del_cdev:
	cdev_del(&aht20->cdev);
del_unregister:
	unregister_chrdev_region(aht20->dev_id, DEVICE_CNT);

	return -EIO;
}
 
static int i2c_drv_remove(struct i2c_client *c)
{
    struct aht20_dev *aht20 = i2c_get_clientdata(c);
	cdev_del(&aht20->cdev);
	unregister_chrdev_region(aht20->dev_id, DEVICE_CNT); 
	device_destroy(aht20->class, aht20->dev_id);
	class_destroy(aht20->class);  
    return 0;
}
 
static struct i2c_driver aht20_drv = {
	.driver = {
		.name	= "aht20_drv",
        .owner = THIS_MODULE,
	},
	.probe		= i2c_drv_probe,
	.remove		= i2c_drv_remove,
	.id_table	= aht20_dev_id,
};
 
static int __init i2c_drv_init(void)
{
	i2c_add_driver(&aht20_drv);
	return 0;
}
 
static void __exit i2c_drv_exit(void)
{
	i2c_del_driver(&aht20_drv);
}
 
module_init(i2c_drv_init);
module_exit(i2c_drv_exit);
MODULE_LICENSE("GPL");