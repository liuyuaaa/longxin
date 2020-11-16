#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct
{
	unsigned char  humi_high8bit;		//ԭʼ���ݣ�ʪ�ȸ�8λ
	unsigned char  humi_low8bit;	 	//ԭʼ���ݣ�ʪ�ȵ�8λ
	unsigned char  temp_high8bit;	 	//ԭʼ���ݣ��¶ȸ�8λ
	unsigned char  temp_low8bit;	 	//ԭʼ���ݣ��¶ȵ�8λ
	unsigned char  check_sum;	 	    //У���
	int    humidity;        //ʵ��ʪ��
	int    temperature;     //ʵ���¶�  
} DHT11_Data_TypeDef;

int main(int argc, char **argv)
{
	int fd;
	int cnt = 0;
	unsigned int val = 1;
	DHT11_Data_TypeDef DHT11_Data;
	
	fd = open("/dev/ds18b20", O_RDWR);
	if (fd < 0)
	{
		printf("can't open\n");
		return -1;
	}
	
	read(fd, &DHT11_Data, sizeof(DHT11_Data_TypeDef));
	printf("humidity = %d\n", DHT11_Data.humidity);
	printf("temperature = %d\n", DHT11_Data.temperature);
	
	return 0;
}


