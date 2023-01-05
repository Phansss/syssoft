
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fd;
	
	fd = open("non_existing_file", O_RDONLY );
	if ( fd == -1 )
	{
		printf("Output of errno:\t\t %d\n", errno);
		
		printf("%s\n", strerror( errno ) );	
		
		perror("My own message before the error string::\t");
	}
	
	return 0;
}

