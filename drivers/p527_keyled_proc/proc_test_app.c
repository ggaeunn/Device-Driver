#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define DEVICE_FILENAME "/proc/keyled/led"

int main(int argc, char *argv[])
{
	int dev;
	char buff[4];

	if(argc < 2)
	{
		printf("Usage : %s [0~127]\n",argv[0]);
		return 1;
	}

	dev = open(DEVICE_FILENAME, O_RDWR );
	if(dev < 0)
	{
		perror("open");
		return 2;
	}
	strcpy(buff,argv[1]);
	write(dev,buff,sizeof(buff));
	printf("write : %s\n",buff);
	read(dev,buff,sizeof(buff));
	printf("read : %s\n",buff);
	close(dev);
	return 0;
}
