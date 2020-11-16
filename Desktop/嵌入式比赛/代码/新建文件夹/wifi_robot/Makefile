KERN_DIR = /Workstation/tools/kernel/linux-3.0.101

all:
	make -C $(KERN_DIR) M=`pwd` modules
	
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

obj-m += motor_drv.o 
obj-m += dht11_drv.o 
gcc:
	mipsel-linux-gcc -o socket socket.c -lpthread
	mipsel-linux-gcc -o test_io_test test_io_test.c
	mipsel-linux-gcc -o test_pwm test_pwm.c
cp:
	cp *.ko socket test_io_test test_pwm /work/nfs_root/
