#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/input.h>

#define KEYINPUT_NAME	"keyinput"
#define KEYINPUT_NUM	4

struct gpio_key_dev{
	struct input_dev *idev;
	struct timer_list timer;
	int key[KEYINPUT_NUM];
	int irq[KEYINPUT_NUM];	
	int index;	
};

struct gpio_key_dev gpio_key;	

int key_array[] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};

static void key_timer_function(struct timer_list *arg)
{
	int val = gpio_get_value(gpio_key.key[gpio_key.index]);
	printk("key_timer_function %d -> %d\n", gpio_key.key[gpio_key.index], val);
	// if(val == 0) {
		input_report_key(gpio_key.idev, key_array[gpio_key.index], !val);
		input_sync(gpio_key.idev);

		// input_report_key(gpio_key.idev, key_array[gpio_key.index], 0);
		// input_sync(gpio_key.idev);
		// printk("key %d press %d\n", gpio_key.key[gpio_key.index], key_array[gpio_key.index]);
	// }
}

static irqreturn_t key_interrupt0(int irq, void *dev_id)
{
	// printk("key_interrupt0 irq %d \n", irq);
	if(irq == gpio_key.irq[0]) {
		mod_timer(&gpio_key.timer, jiffies + msecs_to_jiffies(15));
		gpio_key.index = 0;
	}
    return IRQ_HANDLED;
}

static irqreturn_t key_interrupt1(int irq, void *dev_id)
{
	// printk("key_interrupt1 irq %d \n", irq);
	if(irq == gpio_key.irq[1]) {
		mod_timer(&gpio_key.timer, jiffies + msecs_to_jiffies(15));
		gpio_key.index = 1;
	}
    return IRQ_HANDLED;
}

static irqreturn_t key_interrupt2(int irq, void *dev_id)
{
	// printk("key_interrupt2 irq %d \n", irq);
	if(irq == gpio_key.irq[2]) {
		mod_timer(&gpio_key.timer, jiffies + msecs_to_jiffies(15));
		gpio_key.index = 2;
	}
    return IRQ_HANDLED;
}

static irqreturn_t key_interrupt3(int irq, void *dev_id)
{
	// printk("key_interrupt3 irq %d \n", irq);
	if(irq == gpio_key.irq[3]) {
		mod_timer(&gpio_key.timer, jiffies + msecs_to_jiffies(15));
		gpio_key.index = 3;
	}
    return IRQ_HANDLED;
}

static int __init gpio_key_init(void)
{
	int ret = 0;
	int i = 0;

	for(i = 0; i < KEYINPUT_NUM; i++) {
		// gpio 48 49 50 51
		gpio_key.key[i] = 48 + i;

		// request
		ret = gpio_request(gpio_key.key[i], "LED-GPIO");
		if (ret) {
			printk(KERN_ERR "gpio_key: Failed to request led-gpio\n");
			return ret;
		}

		// input
		ret = gpio_direction_input(gpio_key.key[i]);
		if(ret < 0) {
			printk("can't set gpio!\r\n");
		}

		// irq
		gpio_key.irq[i] = gpio_to_irq(gpio_key.key[i]); 
		printk("key%d -> irq : %d\n", gpio_key.key[i], gpio_key.irq[i]);
		if(i == 0) {
			ret = request_irq(gpio_key.irq[i], key_interrupt0, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "Key0_IRQ", NULL);
			if (ret) {
				gpio_free(gpio_key.key[i]);
				return ret;
			}
		}
		else if(i == 1) {
			ret = request_irq(gpio_key.irq[i], key_interrupt1, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "Key1_IRQ", NULL);
			if (ret) {
				gpio_free(gpio_key.key[i]);
				return ret;
			}
		}
		else if(i == 2) {
			ret = request_irq(gpio_key.irq[i], key_interrupt2, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "Key2_IRQ", NULL);
			if (ret) {
				gpio_free(gpio_key.key[i]);
				return ret;
			}
		}
		else if(i == 3) {
			ret = request_irq(gpio_key.irq[i], key_interrupt3, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "Key3_IRQ", NULL);
			if (ret) {
				gpio_free(gpio_key.key[i]);
				return ret;
			}
		}
	}

	// timer
	timer_setup(&gpio_key.timer, key_timer_function, 0);

	// input dev
	gpio_key.idev = input_allocate_device();
	gpio_key.idev->name = KEYINPUT_NAME;
	gpio_key.idev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	input_set_capability(gpio_key.idev, EV_KEY, KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN);
	ret = input_register_device(gpio_key.idev);
	if (ret) {
		printk("register input device failed!\r\n");
		goto free_gpio;
	}

	return ret;

free_gpio:
	for(i = 0; i < KEYINPUT_NUM; i++) {
		free_irq(gpio_key.irq[i],NULL);
		gpio_free(gpio_key.key[i]);
		
	}

	del_timer_sync(&gpio_key.timer);
	return -EIO;
}

static void __exit gpio_key_exit(void)
{
	int i;
	for(i = 0; i < KEYINPUT_NUM; i++) {
		free_irq(gpio_key.irq[i],NULL);
		gpio_free(gpio_key.key[i]);
	}

	del_timer_sync(&gpio_key.timer);
	input_unregister_device(gpio_key.idev);	
}

module_init(gpio_key_init);
module_exit(gpio_key_exit);
MODULE_LICENSE("GPL");