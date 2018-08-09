#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
int read_n_bytes(int fp, void *dest, int offset, int num_bytes)
{
	/** OFFSET FROM BEGINING OF PARTITION WHERE THE INFO RESIDES */ 
	if (lseek(fp,offset,SEEK_SET)==-1)
	{
		perror("Seek error");
		return -1;
	}
	
	/** READ BYTES */
	if (read(fp,dest,num_bytes)==-1)
	{
		perror("Read error");
		return -1;
	}

	return 0;
}

int write_n_bytes(int fp, void *buf, int offset, int num_bytes) 
{
	/** OFFSET FROM BEGINING OF PARTITION WHERE THE INFO RESIDES */ 
	if (lseek(fp,offset,SEEK_SET)==-1)
	{
		perror("Seek error");
		return -1;
	}

	/** WRITE BYTES */
	if (write(fp, buf, num_bytes))
	{
		perror("Write error");
		return -1;
	}

	return 0;
}

void hexDump(void *source, int size, int start_offset) 
{
	int i,j;
	
	unsigned char *checker = (unsigned char*)malloc(size);
	memcpy(checker, source, size);
	
	/************
	 *  HEADER  *
	 ************/
	int first_col=start_offset & 0xF;
	printf("address   ");
	for(i=first_col; i<first_col+16; i++)
	{
		printf("%2x ", i & 0xF);
		if(((i-first_col)&0xF) == 7) printf(" ");
	}
	printf("  ");
	for(i=first_col; i<first_col+16; i++) 
		printf("%x", i & 0xF);
	printf("\n");
	
	for(i=0; i<size; i++) 
	{
		/********
		 * DATA *
		 ********/
		if((i&0xF)==0) printf("%08x  ", start_offset+i);
		printf("%02x ", checker[i]);
		if((i&0xF) == 7) printf(" ");
		/********************
		 * REFERENCE VALUES *
		 ********************/
		if((i&0xF) == 15) 
		{
			printf(" |");
			for(j=i-15; j<=i; j++) 
			{
				if(checker[j] >= 0x20 && checker[j] <= 0x7E)
					printf("%c", checker[j]);
				else 
					printf(".");
			}
			printf("|\n");
		}
	}
	if((i&0xF))
	{
		for(j=0;j<3*((16-i)&0xF);j++)
		{
			printf(" ");
		}
		if((i&0xF)<8) printf(" ");
		printf(" |");
		for(j=(i&0xFFFFFFF0); j<i; j++) 
		{
			if(checker[j] >= 0x20 && checker[j] <= 0x7E)
				printf("%c", checker[j]);
			else 
				printf(".");
		}
		printf("|\n");
	}
	printf("%08x\n", start_offset+i);
	
	free(checker);
}

void atoi_array(unsigned char *dest, char **src, int size) 
{
	int i;
	for(i=0; i<size; i++) {
		dest[i] = atoi(src[i]);
	}
}
