obj-m += aht20_dev.o aht20_drv.o 
KDIR:=/home/asensing/loongson/linux-4.19
ARCH=loongarch 
CROSS_COMPILE=loongarch64-linux-gnu-
PWD?=$(shell pwd) 
all:
	make -C $(KDIR) M=$(PWD) modules 
