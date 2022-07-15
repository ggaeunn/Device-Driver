#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define DEVICE_FILENAME "/dev/keyled_proc"

int main(int argc, char *argv[])
{
	int dev;
	char buff;
	int ret;
	int num = 0;
	struct pollfd Events[2];
	char keyStr[10];

  	if((argc != 2) || (argv[1][0] != '0') || (!(argv[1][1] == 'x' || argv[1][1] == 'X')))
//	if((argc != 2) || (strncmp(argv[1],"0x",2) && (strncmp(argv[1],"0X",2))))
	{
		printf("Usage : %s [0x00~0x7f]\n",argv[0]);
		return 1;
	}
//	dev = open(DEVICE_FILENAME, O_RDWR | O_NONBLOCK);
	dev = open(DEVICE_FILENAME, O_RDWR );
	if(dev < 0)
	{
		perror("open");
		return 2;
	}
	buff = (char)strtol(argv[1],NULL,16);
	write(dev,&buff,sizeof(buff));
    fflush(stdin);
	while(1)
	{
		memset( Events, 0, sizeof(Events));
		Events[0].fd = dev;
		Events[0].events = POLLIN;
		Events[1].fd = fileno(stdin);
		Events[1].events = POLLIN;

		ret = poll(Events, 2, 1000);
		if(!ret)
		{
//  			printf("poll time out : %d\n",num++);
			continue;
		}
		if(Events[0].revents & POLLIN)
		{
			ret = read(dev,&buff,sizeof(buff));
  			if(buff == 0)
			{
				printf("TEST continue : %d\n",num++);
  				continue;
			}
			printf("TEST buff : %d\n",buff);
			write(dev,&buff,sizeof(buff));
			if(buff == 0x02)
				break;
		}
		else if(Events[1].revents & POLLIN)
		{
			fgets(keyStr,sizeof(keyStr),stdin);
			if(keyStr[0] == 'q')
				break;
			keyStr[strlen(keyStr)-1] = '\0';
			printf("STDIN : %s\n",keyStr);
			buff = atoi(keyStr);
			write(dev,&buff,sizeof(buff));
		}
	}
	close(dev);
	return 0;
}
