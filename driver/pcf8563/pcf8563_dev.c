#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>

static struct i2c_board_info    pcf8563;
static struct i2c_client *      client;

#define DEV_ID_NAME "loongson,pcf8563"

static const unsigned short addrs[] = {0x51, I2C_CLIENT_END};

static int dev_init(void)
{
    struct i2c_adapter *adapter = NULL;

    memset(&pcf8563, 0, sizeof(struct i2c_board_info));
    strlcpy(pcf8563.type, DEV_ID_NAME, I2C_NAME_SIZE);

    adapter = i2c_get_adapter(0);
 
    client = i2c_new_probed_device(adapter, &pcf8563, addrs, NULL);

    i2c_put_adapter(adapter);

    if (client)
    {
        return 0;
    }
    else
    {
        return -ENODEV;
    }
}

static void dev_exit(void)
{
    i2c_unregister_device(client);
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");