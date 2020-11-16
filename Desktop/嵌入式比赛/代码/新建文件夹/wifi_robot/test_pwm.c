#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	int fd;
	int cnt = 0;
	int val = 1;
	
	fd = open("/dev/motor", O_RDWR);
	if (fd < 0)
	{
		printf("can't open\n");
		return -1;
	}
	val = atoi(argv[1]);
	write(fd, &val, sizeof(val));
	printf("val = %d\n", val);
	
	return 0;
}


