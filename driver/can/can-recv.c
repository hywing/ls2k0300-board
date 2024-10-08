#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if.h>
 
int main() {
    // 创建CAN套接字
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("socket");
        return 1;
    }
 
    // 绑定到can0接口
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        return 1;
    }
 
    // 配置CAN套接字地址
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
 
    // 绑定套接字到指定的CAN接口
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
 
    // 设置接收过滤器（如果需要）
#if 0
    // 发送CAN消息
    struct can_frame frame;
    frame.can_id = 0x123; // CAN标识符
    frame.can_dlc = 2;    // 数据长度
    frame.data[0] = 0x01; // 数据字节1
    frame.data[1] = 0x23; // 数据字节2
    if (write(s, &frame, sizeof(frame)) < 0) {
        perror("write");
        return 1;
    }

    // 接收CAN消息
    struct can_frame rframe;
    int nbytes;
    while ((nbytes = read(s, &rframe, sizeof(rframe))) > 0) {
        // 处理接收到的消息
        printf("Received CAN message: ID=%x, DLC=%d, Data=%02x %02x\n",
               rframe.can_id, rframe.can_dlc, rframe.data[0], rframe.data[1]);
    }
#else

    // 发送CAN消息
    // struct can_frame frame;
    // frame.can_id = 0x123; // CAN标识符
    // frame.can_dlc = 2;    // 数据长度
    // frame.data[0] = 0x01; // 数据字节1
    // frame.data[1] = 0x23; // 数据字节2

    // 接收CAN消息
    struct can_frame rframe;
    int nbytes;

    while ((nbytes = read(s, &rframe, sizeof(rframe))) > 0) {
        // 处理接收到的消息
        printf("Received CAN message: ID=0x%x, DLC=%d, Data=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
               rframe.can_id, rframe.can_dlc, rframe.data[0], rframe.data[1], rframe.data[2], rframe.data[3], rframe.data[4], rframe.data[5], rframe.data[6], rframe.data[7]);
    }
#endif
    // 关闭套接字
    close(s);
    return 0;
}
