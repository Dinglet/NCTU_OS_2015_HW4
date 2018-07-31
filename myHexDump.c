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
	int fp, start, end, size;
	unsigned char *buffer;
	
	if(argc != 4)
	{
		perror("Usage: ./myHexDump IMAGE START OFFSET");
	}
	
	start = strtoul(argv[2],NULL,0); // byte offset
	end = strtoul(argv[2],NULL,0)+strtoul(argv[3],NULL,0);
	size = end - start;
	
	buffer = (unsigned char*) malloc(size);
	if(buffer == NULL) 
	{
		perror("Error allocating memory");
		exit(-1);
	}
	
	/** OPEN the looper or images using System API */
	/** REMIND: you will need O_NONBLOCK as flag */
		/** why??? */
	if ((fp = open(argv[1], O_RDONLY|O_NONBLOCK)) == -1)
	{
		perror("Error opening image");
		free(buffer);
		exit(-1);
	}
	
	if(read_n_bytes(fp, buffer, size, start) == -1)
	{
		perror("Error reading image");
		free(buffer);
		exit(-1);
	}
	
	hexDump(buffer, size, start);
	
	close(fp);
	
	free(buffer);
	
	return 0;
}
