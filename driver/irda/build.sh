export PATH=$PATH:/home/asensing/loongson/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.3-1/bin
make -j8
# loongarch64-linux-gnu-gcc test.c -o test
FILE=$PWD/$(basename $PWD).ko
scp $FILE root@192.168.137.232:/home/root