#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>  
#include <unistd.h>

#define false 0
#define true 1
#define bool char
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 
#define gpsRxBufferLength  76

typedef struct 
{
	char GPS_Buffer[GPS_Buffer_Length];
	char UTCTime[UTCTime_Length];		//UTC时间
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
}GnssPacket;


static unsigned char gpsRxBuffer[gpsRxBufferLength];
static unsigned char RX_Count;
static GnssPacket gnss;
static bool isGetData = false;
static bool isParseData = false;
static bool isUsefull = false;

void Delay_ms(unsigned int n)
{
	usleep(n*1000);
}

void errorLog(int num)
{
	
	while (1)
	{
	  	printf("ERROR : %c\r\n", num+0x30);
	}
}

void clrStruct()
{
	isGetData = false;
	isParseData = false;
	isUsefull = false;
	memset(gnss.GPS_Buffer, 0, GPS_Buffer_Length);      //���
	memset(gnss.UTCTime, 0, UTCTime_Length);
	memset(gnss.latitude, 0, latitude_Length);
	memset(gnss.N_S, 0, N_S_Length);
	memset(gnss.longitude, 0, longitude_Length);
	memset(gnss.E_W, 0, E_W_Length);
	
}
 
void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (isGetData)
	{
		isGetData = false;
		printf("**************************************************************************************************\r\n");
		printf(gnss.GPS_Buffer);

		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(gnss.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//��������
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(gnss.UTCTime, subString, subStringNext - subString);break;	//��ȡUTCʱ��
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//��ȡUTCʱ��
						case 3:memcpy(gnss.latitude, subString, subStringNext - subString);break;	//��ȡγ����Ϣ
						case 4:memcpy(gnss.N_S, subString, subStringNext - subString);break;	//��ȡN/S
						case 5:memcpy(gnss.longitude, subString, subStringNext - subString);break;	//��ȡ������Ϣ
						case 6:memcpy(gnss.E_W, subString, subStringNext - subString);break;	//��ȡE/W

						default:break;
					}

					subString = subStringNext;
					isParseData = true;
					if(usefullBuffer[0] == 'A')
						isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						isUsefull = false;

				}
				else
				{
					errorLog(2);	//��������
				}
			}


		}
	}
}

void printGpsBuffer()
{
	if (isParseData)
	{
		isParseData = false;
		
		printf("gnss.UTCTime = ");
		printf(gnss.UTCTime);
		printf("\r\n");

		if(isUsefull)
		{
			isUsefull = false;
			printf("gnss.latitude = ");
			printf(gnss.latitude);
			printf("\r\n");


			printf("gnss.N_S = ");
			printf(gnss.N_S);
			printf("\r\n");

			printf("gnss.longitude = ");
			printf(gnss.longitude);
			printf("\r\n");

			printf("gnss.E_W = ");
			printf(gnss.E_W);
			printf("\r\n");
		}
		else
		{
			printf("GNSS DATA is not usefull!\r\n");
		}
		
	}
}

void getGNSSData(unsigned char SBUF)
{
    unsigned char temp = 0;
	char i = 0;

	temp = SBUF;

	
	if(temp == '$')
	{
		RX_Count = 0;	
	}
		
	if(RX_Count <= 5)
	{
	   gpsRxBuffer[RX_Count++] = temp;
	}
	else if(gpsRxBuffer[0] == '$' &gpsRxBuffer[4] == 'M' && gpsRxBuffer[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
	{
		gpsRxBuffer[RX_Count++] = temp;
		if(temp == '\n')									   
		{
			memset(gnss.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
			memcpy(gnss.GPS_Buffer, gpsRxBuffer, RX_Count); 	//保存数据
			isGetData = true;
			RX_Count = 0;
			memset(gpsRxBuffer, 0, gpsRxBufferLength);      //清空				
		}
		
		if(RX_Count >= 75)
		{
			RX_Count = 75;
			gpsRxBuffer[RX_Count] = '\0';//添加结束符
		}			
	}
}

 
int main(int argc, char const *argv[])
{
    // uart fd
    int fd = open("/dev/ttyS2", O_RDONLY);  
    if (fd == -1) {  
        perror("open_port: Unable to open /dev/ttyS2");  
        return -1;  
    }

    // uart config
    struct termios tty;  
    memset(&tty, 0, sizeof tty);  
    
    if(tcgetattr(fd, &tty) != 0) {  
        perror("Error from tcgetattr");  
        return -1;  
    }  
    
    // cfsetospeed(&tty, B9600);  
    cfsetispeed(&tty, B9600);  
    
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
        // 读取数据  
        char buf[512];  
        int num_bytes = read(fd, buf, sizeof(buf));  
        if (num_bytes < 0) {  
            perror("Error reading from serial port");  
            return -1;  
        }  
        buf[num_bytes] = '\0'; // 添加字符串结束符  
        
        size_t len = strlen(buf);
        for(int i = 0; i < len; i++) {
            getGNSSData(buf[i]);
        }

        parseGpsBuffer();

        printGpsBuffer();
        sleep(1);
    }

    close(fd);
    return 0;
}
