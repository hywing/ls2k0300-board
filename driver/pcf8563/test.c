#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"

#define DEV_NAME "/dev/aht20"

int main()
{
    int fd, temp, humi;
    unsigned int data[2];

    fd = open(DEV_NAME, 0);
    if(fd < 0)
    {
        printf("Open %s failed\n", DEV_NAME);
        return 1;
    }
    else
	{
		printf("Open %s success!\n", DEV_NAME);
	}

    while(1)
    {
        read(fd, &data, sizeof(data)); 

		humi = data[0] * 1000.0 / 1024 / 1024;  			
        temp = data[1] * 2000.0 / 1024 / 1024 - 500;

		printf("temp : %d.%d℃, humi : %d.%d%%\n", (temp/10), (temp%10), (humi/10),(humi%10));

		sleep(1);
    }

	close(fd);
	return 0;
}
