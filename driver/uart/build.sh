export PATH=$PATH:/home/asensing/loongson/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.3-1/bin
loongarch64-linux-gnu-gcc uart_send.c -o uart_send
loongarch64-linux-gnu-gcc uart_recv.c -o uart_recv
scp uart_send uart_recv root@192.168.137.10:/home/root