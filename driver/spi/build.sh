export PATH=$PATH:/home/asensing/loongson/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.3-1/bin
export ARCH=loongarch 
export CROSS_COMPILE=loongarch64-linux-gnu-
loongarch64-linux-gnu-gcc main.c lcd_init.c lcd.c -o spi-test
scp spi-test root@192.168.137.188:/home/root