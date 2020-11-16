#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define SERVER_PORT 8888
#define BACKLOG     10

#define WIFI_ROBOT_RUN		_IO('c',0x01)
#define WIFI_ROBOT_BACK	 	_IO('c',0x02)
#define WIFI_ROBOT_LEFT		_IO('c',0x03)
#define WIFI_ROBOT_RIGHT	_IO('c',0x04)
#define WIFI_ROBOT_STOP	 	_IO('c',0x05)
#define WIFI_BUZZER_ON		_IO('c',0x06)
#define WIFI_BUZZER_OFF		_IO('c',0x07)
#define WIFI_ROBOT_RATE_L	_IO('c',0x08)
#define WIFI_ROBOT_RATE_R   _IO('c',0x09)

#define TRUE    1
#define FALSE   0

int fd;
int temp_fd;
int iSocketServer;

typedef struct
{
	unsigned char  humi_high8bit;		//原始数据：湿度高8位
	unsigned char  humi_low8bit;	 	//原始数据：湿度低8位
	unsigned char  temp_high8bit;	 	//原始数据：温度高8位
	unsigned char  temp_low8bit;	 	//原始数据：温度低8位
	unsigned char  check_sum;	 	    //校验和
	long  flag; 
} DHT11_Data_TypeDef;


void sigroutine(int dunno)
{ 
	printf("receive ctrl C\n");
	close(iSocketServer);
	exit(0);
}

static int is_direction(unsigned char *string)
{
	if(strncmp(string, "direction", 9) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static int is_temp(unsigned char *string)
{
	if(strncmp(string, "temp", 4) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static int is_buzzer(unsigned char *string)
{
	if(strncmp(string, "buzzer", 6) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static int is_speed(unsigned char *string)
{
	if(strncmp(string, "speed", 5) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static void direction_control(unsigned char *string)
{
	if ( strstr(string, "run") != NULL )
	{
		ioctl(fd, WIFI_ROBOT_RUN);
		printf("run\n");
	}
	else if ( strstr(string, "back") != NULL )
	{
		ioctl(fd, WIFI_ROBOT_BACK);
		printf("back\n");
	}
	else if ( strstr(string, "left") != NULL )
	{
		ioctl(fd, WIFI_ROBOT_LEFT);
		printf("left\n");
	}
	else if ( strstr(string, "right") != NULL )
	{
		ioctl(fd, WIFI_ROBOT_RIGHT);
		printf("right\n");
	}
	else if ( strstr(string, "rate_l") != NULL )
	{
		ioctl(fd, WIFI_ROBOT_RATE_L);
		printf("rate_l\n");
	}
	else if ( strstr(string, "rate_r") != NULL )
	{
		ioctl(fd, WIFI_ROBOT_RATE_R);
		printf("rate_r\n");
	}
	else
	{
		ioctl(fd, WIFI_ROBOT_STOP);
		printf("stop\n");
	}
}

static void buzzer_control(unsigned char *string)
{
	static int a = 1;
	if ( a == 1 )
	{
		ioctl(fd, WIFI_BUZZER_ON);
		a = 0;
	}
	else
	{
		ioctl(fd, WIFI_BUZZER_OFF);
		a = 1;
	}
}

static void speed_control(unsigned char *string)
{
	unsigned char *pb = string;
	int val;
	pb = strstr(string, "speed:");
	if(NULL != pb)
	{
		pb = strchr(pb, ':');
		pb++;
		val = atol(pb)+35;
		write(fd, &val, sizeof(val));
	}
}

static void get_temperature(int *SocketClient)
{
	char ucSendBuf[20] = {0};
	int iSendLen;
	float humidi;
	float temper;
	
	static int cnt;
	
	int n = 3;
	while (n!=0)
	{
		DHT11_Data_TypeDef DHT11_Data = {41, 11, 19, 11};

			
		read(temp_fd, &DHT11_Data, sizeof(DHT11_Data_TypeDef));
		humidi = (float)(DHT11_Data.humi_high8bit*100+DHT11_Data.humi_low8bit)/100; 
		temper = (float)(DHT11_Data.temp_high8bit*100+DHT11_Data.temp_low8bit)/100;
		n--;
		if (DHT11_Data.flag == 111)
			break;
	}
	
	sprintf(ucSendBuf, "hum=%.2lf, tem=%.2lf", humidi, temper);
	printf("%s\n",ucSendBuf);

	iSendLen = send(*SocketClient, ucSendBuf, strlen(ucSendBuf), 0);
	if (iSendLen <= 0)
	{
		printf("send temperature error\n");
		close(*SocketClient);
		return;
	}
}

int main(int argc, char **argv)
{
	pthread_t Duty_Cycle_Id;
	int result;

	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	struct sockaddr_in tSocketClientAddr;
	int iRet;
	int iAddrLen;

	int iRecvLen;
	unsigned char ucRecvBuf[100];

	int iClientNum = -1;
	
	signal(SIGCHLD,SIG_IGN);
	signal(SIGINT, sigroutine);
	
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == iSocketServer)
	{
		printf("socket error!\n");
		return -1;
	}

	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);  /* host to net, short */
 	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset(tSocketServerAddr.sin_zero, 0, 8);
	
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		printf("bind error!\n");
		return -1;
	}

	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}

	fd = open("/dev/motor", O_RDWR);
	if (fd < 0)
	{
		printf("error, can't open /dev/motor\n");
		return 0;
	}

	temp_fd = open("/dev/dht11", O_RDWR);
	if (fd < 0)
	{
		printf("error, can't open /dev/dht11\n");
		return 0;
	}
	while (1)
	{
		iAddrLen = sizeof(struct sockaddr);
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		if (-1 != iSocketClient)
		{
			iClientNum++;
			printf("connect from client %d : %s\n",  iClientNum, inet_ntoa(tSocketClientAddr.sin_addr));
			if (!fork())
			{
				while (1)
				{
					memset(ucRecvBuf, 0, 100);
					
					iRecvLen = recv(iSocketClient, ucRecvBuf, 100, 0);
					if (iRecvLen <= 0)
					{
						close(iSocketClient);
						return -1;
					}
					else
					{
						if(is_direction(ucRecvBuf) == TRUE)
						{
							direction_control(ucRecvBuf);
						}
						else if(is_buzzer(ucRecvBuf) == TRUE)
						{
							buzzer_control(ucRecvBuf);
						}
						else if(is_speed(ucRecvBuf) == TRUE)
						{
							speed_control(ucRecvBuf);
							printf("speed\n");
						}
						else if(is_temp(ucRecvBuf) == TRUE)
						{
							get_temperature(&iSocketClient);
							printf("temp\n");
						}
						
					}
				}				
			}
		}
	}
	pthread_join(Duty_Cycle_Id, (void *)&result);
	close(iSocketServer);
	return 0;
}


