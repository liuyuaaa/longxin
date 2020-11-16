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

#define WIFI_ROBOT_RUN		_IO('c',0x01)
#define WIFI_ROBOT_BACK	 	_IO('c',0x02)
#define WIFI_ROBOT_LEFT		_IO('c',0x03)
#define WIFI_ROBOT_RIGHT	_IO('c',0x04)
#define WIFI_ROBOT_STOP	 	_IO('c',0x05)
#define WIFI_BUZZER_ON		_IO('c',0x06)
#define WIFI_BUZZER_OFF		_IO('c',0x07)
#define WIFI_ROBOT_RATE_L	_IO('c',0x08)
#define WIFI_ROBOT_RATE_R   _IO('c',0x09)

static struct class *seconddrv_class;
static struct class_device	*seconddrv_class_dev;


#define AIN1 47
#define AIN2 1
#define BIN1 2
#define BIN2 57
#define STBY 55
#define PWM0 72
#define PWM1 70
#define BUZZER 3

#define PWM0_CNTR 0xbfe5c000
#define PWM0_HRC  0xbfe5c004
#define PWM0_LRC  0xbfe5c008
#define PWM0_CTRL 0xbfe5c00c

#define PWM1_CNTR 0xbfe5c010
#define PWM1_HRC  0xbfe5c014
#define PWM1_LRC  0xbfe5c018
#define PWM1_CTRL 0xbfe5c01c


static int wifi_robot_motor_control_open(struct inode *inode, struct file *file)
{
	gpio_direction_output(AIN1, 1);
	gpio_direction_output(AIN2, 1);
	gpio_direction_output(BIN1, 1);
	gpio_direction_output(BIN2, 1);
	gpio_direction_output(STBY, 1);
	gpio_direction_output(PWM0, 1);
	gpio_direction_output(PWM1, 1);
	gpio_direction_output(BUZZER, 0);
	
	return 0;
}

void pwn_init()
{
	__raw_writel(0, PWM0_CNTR); 
	__raw_writel(30, PWM0_HRC); 
	__raw_writel(40, PWM0_LRC); 
	__raw_writel(1, PWM0_CTRL); 

	__raw_writel(0, PWM1_CNTR); 
	__raw_writel(30, PWM1_HRC); 
	__raw_writel(40, PWM1_LRC); 
	__raw_writel(1, PWM1_CTRL); 
	
}

static ssize_t wifi_robot_motor_control_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{

	int val;
	copy_from_user(&val, buf, 4);
	if (val > 0)
	{
		__raw_writel(0, PWM0_CTRL);
		__raw_writel(0, PWM0_CNTR);
		__raw_writel(30, PWM0_HRC); 
		__raw_writel(val*2, PWM0_LRC);
		__raw_writel(1, PWM0_CTRL);

		__raw_writel(0, PWM1_CTRL);
		__raw_writel(0, PWM1_CNTR);
		__raw_writel(30, PWM1_HRC); 
		__raw_writel(val*2, PWM1_LRC);
		__raw_writel(1, PWM1_CTRL);		 
	}
	printk("linux val = %d\n", val);	
}

static int wifi_robot_motor_control_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
		case WIFI_ROBOT_RUN :		// Ç°½ø,µÆ²»ÁÁ
		{
			gpio_set_value(AIN1, 1);
			gpio_set_value(AIN2, 0);

			gpio_set_value(BIN1, 1);
			gpio_set_value(BIN2, 0);
				
			break;
		}

		case WIFI_ROBOT_BACK :		// ºóÍË,µÆ¶¼ÁÁ
		{
			gpio_set_value(AIN1, 0);
			gpio_set_value(AIN2, 1);

			gpio_set_value(BIN1, 0);
			gpio_set_value(BIN2, 1);
			
			break;
		}

		case WIFI_ROBOT_LEFT :		// ×ó×ªÍä,×ó±ßµÆÁÁ
		{
			
			gpio_set_value(AIN1, 1);
			gpio_set_value(AIN2, 0);
			
			gpio_set_value(BIN1, 0);
			gpio_set_value(BIN2, 1);
			mdelay(20);
			gpio_set_value(BIN1, 1);
			gpio_set_value(BIN2, 0);
			break;
		}

		case WIFI_ROBOT_RIGHT :		// ÓÒ×ªÍä,ÓÒ±ßµÆÁÁ
		{
			gpio_set_value(BIN1, 1);
			gpio_set_value(BIN2, 0);
			
			gpio_set_value(AIN1, 0);
			gpio_set_value(AIN2, 1);
			mdelay(20);
			gpio_set_value(AIN1, 1);
			gpio_set_value(AIN2, 0);
			break;
		}

		case WIFI_ROBOT_STOP :		// Í£Ö¹,µÆ¶¼ÁÁ
		{
			gpio_set_value(AIN1, 1);
			gpio_set_value(AIN2, 1);

			gpio_set_value(BIN1, 1);
			gpio_set_value(BIN2, 1);
			
			break;
		}
		
		case WIFI_ROBOT_RATE_L :		// Ç°½ø,µÆ²»ÁÁ
		{
			gpio_set_value(AIN1, 1);
			gpio_set_value(AIN2, 0);

			gpio_set_value(BIN1, 0);
			gpio_set_value(BIN2, 1);
				
			break;
		}
		
		case WIFI_ROBOT_RATE_R :		// Ç°½ø,µÆ²»ÁÁ
		{
			gpio_set_value(AIN1, 0);
			gpio_set_value(AIN2, 1);

			gpio_set_value(BIN1, 1);
			gpio_set_value(BIN2, 0);
				
			break;
		}
		
		case WIFI_BUZZER_ON :		// ·äÃùÆ÷Ïì
		{
			gpio_set_value(BUZZER, 1);
			
			break;
		}

		case WIFI_BUZZER_OFF :		// ·äÃùÆ÷²»Ïì
		{
			gpio_set_value(BUZZER, 0);
			break;
		}
		
		default :
			break;
	}

	return 0;
}

static struct file_operations sencod_drv_fops = {
	.owner		= THIS_MODULE, 
	.open		= wifi_robot_motor_control_open,
	.write		= wifi_robot_motor_control_write,
	.unlocked_ioctl 	= wifi_robot_motor_control_ioctl,
};

int major;
static int wifi_robot_motor_control_init(void)
{
	major = register_chrdev(0, "wifi_robot_motor_control", &sencod_drv_fops);

	seconddrv_class = class_create(THIS_MODULE, "wifi_robot_motor_control");
	seconddrv_class_dev = device_create(seconddrv_class, NULL, MKDEV(major, 0), NULL, "motor"); 

	pwn_init();
	
	return 0;
}

static void wifi_robot_motor_control_exit(void)
{
	unregister_chrdev(major, "wifi_robot_motor_control");
	device_unregister(seconddrv_class_dev);
	class_destroy(seconddrv_class);
}

module_init(wifi_robot_motor_control_init);
module_exit(wifi_robot_motor_control_exit);

MODULE_LICENSE("GPL");

