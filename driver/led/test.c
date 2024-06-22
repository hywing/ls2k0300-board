#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

int main(int argc, char *argv[])
{
    int fd, ret;
    char *filename;
    unsigned char data[1];

    if(argc != 2){
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("file %s open failed!\r\n", argv[1]);
        return -1;
    }

    while(1) {
        data[0] = 1;
        ret = write(fd, data, sizeof(data));
        if(ret < 0){
            printf("LED Control Failed!\r\n");
            close(fd);
            return -1;
        }
        usleep(500000);

        data[0] = 0;
        ret = write(fd, data, sizeof(data));
        if(ret < 0){
            printf("LED Control Failed!\r\n");
            close(fd);
            return -1;
        }
        usleep(500000);
    }

    ret = close(fd);
    if(ret < 0){
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}