
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define CODE_SIZE 4
#define HX1838_IO 48
#define TIME_UNIT 140
#define GPIO_VALUE gpio_get_value(HX1838_IO)

typedef unsigned char uchar;

static uchar array[CODE_SIZE];

irqreturn_t irq_func(int irqno, void *arg)
{
    unsigned char j,k,N=0;

    udelay(15 * TIME_UNIT);

    if (GPIO_VALUE == 1)
    {
        // printk("111\n");
        return IRQ_HANDLED;
    }
    
    while (!GPIO_VALUE)  
    {
        udelay(TIME_UNIT);
    }                                       
        
    for (j=0; j < 4; j++)                                          
    {
        for (k = 0; k < 8; k++)                                
        {
            while (GPIO_VALUE)                                  
            {
                udelay(TIME_UNIT);
            }

            while (!GPIO_VALUE)                                
            {
                udelay(TIME_UNIT);
            }

            while (GPIO_VALUE)                                  
            {
                udelay(TIME_UNIT);
                N++;           
                if (N >= 30)
                {
                    // printk("222\n");
                    return IRQ_HANDLED;
                }                                              
            } 
                                                                          
            array[j] = array[j] >> 1;                            
            if(N >= 8)
            {
                array[j] = array[j] | 0x80;                     
            }  
            N = 0;
        }
    }      

    if(array[2] != (unsigned char)(~array[3]))
    {
        // printk("333\n");
        return IRQ_HANDLED;
    }

    printk("%d,%d,%d,%d\n", array[0], array[1], array[2], array[3]);

    return IRQ_HANDLED;
}

static int __init hx1838_init(void)
{
    int ret;
    int irq = gpio_to_irq(HX1838_IO);

    printk("irq : %d\n", irq);

	gpio_request(HX1838_IO, "hx1838-gpio");

    gpio_direction_input(HX1838_IO);

    ret = request_irq(irq, irq_func, IRQF_TRIGGER_FALLING, "hx1838-irq", NULL);

    if (ret < 0)
        goto LABEL;

    return 0;

LABEL:
    gpio_free(HX1838_IO);
    return ret;
}


static void __exit hx1838_exit(void)
{
    free_irq(gpio_to_irq(HX1838_IO), NULL);
    gpio_free(HX1838_IO);
}

module_init(hx1838_init);
module_exit(hx1838_exit);

MODULE_LICENSE("GPL");
