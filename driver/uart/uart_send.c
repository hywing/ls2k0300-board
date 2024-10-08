#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>  
 
 
 
int main(int argc, char const *argv[])
{
    // uart fd
    int fd = open("/dev/ttyS1", O_WRONLY);  
    if (fd == -1) {  
        perror("open_port: Unable to open /dev/ttyS1");  
        return -1;  
    }

    // uart config
    struct termios tty;  
    memset(&tty, 0, sizeof tty);  
    
    if(tcgetattr(fd, &tty) != 0) {  
        perror("Error from tcgetattr");  
        return -1;  
    }  
    
    cfsetospeed(&tty, B9600);  
    // cfsetispeed(&tty, B9600);  
    
    tty.c_cflag &= ~PARENB; // 清除校验位  
    tty.c_cflag &= ~CSTOPB; // 停止位为1  
    tty.c_cflag &= ~CSIZE;  
    tty.c_cflag |= CS8; // 8位数据位  
    tty.c_cflag &= ~CRTSCTS; // 无硬件流控  
    tty.c_cflag |= CREAD | CLOCAL; // 打开接收器，忽略modem控制线  
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 原始输入  
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用软件流控  
    tty.c_oflag &= ~OPOST; // 原始输出  
    tty.c_cc[VMIN] = 0; // 读取时不等待  
    tty.c_cc[VTIME] = 5; // 0.5秒超时  
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {  
        perror("Error from tcsetattr");  
        return -1;  
    }

    while(1) {
        // 写入数据  
        char *data = "AT+VERSION";  
        int len = write(fd, data, strlen(data));  
        if (len < 0) {  
            perror("Error writing to serial port");  
            return -1;  
        } 
        printf("len : %d\n", len);
        sleep(1);
    }

#if 0
    // 读取数据  
    char buf[256];  
    int num_bytes = read(fd, buf, sizeof(buf));  
    if (num_bytes < 0) {  
        perror("Error reading from serial port");  
        return -1;  
    }  
    buf[num_bytes] = '\0'; // 添加字符串结束符  
    printf("Read %d bytes: %s\n", num_bytes, buf);
 #endif

    close(fd);
    return 0;
}
