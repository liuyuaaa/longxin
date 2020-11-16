#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <platform.h>
#include <loongson1.h>
#include <linux/gpio.h>

static struct class *seconddrv_class;
static struct device *seconddrv_class_dev;

#define DS18B20_Pin 51
//定义DS18B20端口控制
#define   DS18B20_L    	gpio_set_value(DS18B20_Pin, 0)    	 	//拉低数据线电平
#define   DS18B20_H    	gpio_set_value(DS18B20_Pin, 1)      		//拉高数据线电平
#define   DS18B20_OUT	gpio_direction_output(DS18B20_Pin, 1)			//数据线设置为输出
#define   DS18B20_IN		gpio_direction_input(DS18B20_Pin)		//数据线设置为输入
#define   DS18B20_STU	gpio_get_value(DS18B20_Pin)       	//数据状态

typedef struct
{
	unsigned char  humi_high8bit;		//原始数据：湿度高8位
	unsigned char  humi_low8bit;	 	//原始数据：湿度低8位
	unsigned char  temp_high8bit;	 	//原始数据：温度高8位
	unsigned char  temp_low8bit;	 	//原始数据：温度低8位
	unsigned char  check_sum;	 	    //校验和
	long  flag; 
} DHT11_Data_TypeDef;


#define READ 0
#define SEND 1

char buf1[10];
int k = 0;

char buf[10];
int z = 0;

unsigned char time1;

static void DHT11_Init(unsigned char select)
{
	if(select == READ){
		DS18B20_IN;
	}else if(select == SEND){
		DS18B20_OUT;
	}
}

int DHT11_Get(DHT11_Data_TypeDef *DHT11_Data)
{
	unsigned char data[5] = {0};
	unsigned char i,j;
	//float temperature,humidity;
	
	int k = 0;
	int z = 0;
	
	DHT11_Init(SEND);
	DS18B20_H;
	//1s
	mdelay(500);
	//起始信号
	DS18B20_L;
	//20ms
	mdelay(20);
	DS18B20_H;
	udelay(30);
	DHT11_Init(READ);
	
	time1 = 1;
	//等待低电平
	while(DS18B20_STU && (time1++))
	{
		udelay(2);
		if(time1 > 200)
		break;
	}
	
	//83us低电平到来，等待高电平
	time1 = 1;
	while((!DS18B20_STU) && (time1++))
	{
		udelay(2);
		if(time1 > 200)
		break;
	}
			
	for(i=0;i<5;i++){
		for(j=0;j<8;j++){
			//先等待低电平到来
			
			time1 = 1;
			while(DS18B20_STU && (time1++))
			{
				udelay(1);
				if(time1 > 150)
				break;
			}
			//54us低电平到来,
			//等待高电平到来
			time1 = 1;
			while((!DS18B20_STU) && (time1++))
			{
				udelay(1);
				if(time1 > 150)
				break;
			}
			//buf[z++] = time1;
			//延时27us以上,判断数据位,0或1
			udelay(25);
			//先把数据左移1位
			data[i] = data[i] << 1;
			//判断40us后是否还为高电平,是数据为1,否为0
			if (DS18B20_STU)
				data[i] |= 0x01;
			//一位数据接收完毕
			//等待高电平结束,下一个数据低电平的到来
		}
		buf1[k++] = data[i];
	}
	//判断校验位
	if(data[4] == (data[0]+data[1]+data[2]+data[3])){
		
		DHT11_Data->humi_high8bit = data[0];
		DHT11_Data->humi_low8bit = data[1];
		DHT11_Data->temp_high8bit = data[2];
		DHT11_Data->temp_low8bit = data[3];
		/*for (k = 0; k < 5; k++)
		{
			printk("buf1[%d] = %d\n", k, buf1[k]);
		}*/
		DHT11_Data->flag = 0;
		printk("success\n");
		return 1;
	}
/*
	for (z = 0; z < 40; z++)
        {
                printk("[%d] = %d  ", z ,buf[z]);
                if (((z+1)%8) == 0)
                        printk("\n");
        }

	*/	
	/*for (k = 0; k < 5; k++)
	{
		printk("buf1[%d] = %d\n", k, buf1[k]);
	}*/
	return 0;
}

static int wifi_robot_motor_control_open(struct inode *inode, struct file *file)
{
	DS18B20_OUT;
	
	return 0;
}

static ssize_t wifi_robot_motor_control_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	int tmp;
	DHT11_Data_TypeDef DHT11_Data;
	DHT11_Data.flag = 0;
	tmp = DHT11_Get(&DHT11_Data);
	if (tmp == 1)
		copy_to_user(buf, &DHT11_Data, sizeof(DHT11_Data_TypeDef));    //将读取得的DS18B20数值复制到用户区
	
	return 0;
}

static struct file_operations sencod_drv_fops = {
	.owner		= THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open		= wifi_robot_motor_control_open,
	.read		= wifi_robot_motor_control_read,
};

static int major;
static int wifi_robot_ds18b20_control_init(void)
{
	major = register_chrdev(0, "wifi_robot_dht11_control", &sencod_drv_fops);

	seconddrv_class = class_create(THIS_MODULE, "wifi_robot_ds18b20_control");
	seconddrv_class_dev = device_create(seconddrv_class, NULL, MKDEV(major, 0), NULL, "dht11"); /* /dev/dht11 */
	
	return 0;
}

static void wifi_robot_ds18b20_control_exit(void)
{
	unregister_chrdev(major, "wifi_robot_dht11_control");
	device_unregister(seconddrv_class_dev);
	class_destroy(seconddrv_class);
}

module_init(wifi_robot_ds18b20_control_init);
module_exit(wifi_robot_ds18b20_control_exit);

MODULE_LICENSE("GPL");
