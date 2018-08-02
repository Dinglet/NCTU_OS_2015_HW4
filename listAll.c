#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include <common.h>

int main(int argc, char **argv) 
{
	int fd;
    const char *device_name;
	
	if(argc < 4)
	{
		perror("Usage: ./listAll IMAGE");
		exit(-1);
	}
	
	device_name = argv[1];
	
	/** OPEN the looper or images using System API */
	if ((fd = open(device_name, O_RDONLY|O_NONBLOCK)) == -1)
	{
		perror("Error opening image");
		exit(-1);
	}
	

	close(fd);

	return 0;
}
