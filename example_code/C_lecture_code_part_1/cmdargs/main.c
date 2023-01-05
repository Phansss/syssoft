
#include <stdio.h>
#include <stdlib.h>


/*
int main(int argc, char *argv[])
{
	int counter;
	printf("The arguments to the program are:\n");

    //1st solution:
	for ( counter = 0; counter < argc; counter++)
	  printf("argument %d = %s\n", counter, argv[counter] );  //or: *(argv+counter)
	return 0;
}

*/
// OR .....


int main(int argc, char **argv)
{
	int counter;
	printf("The arguments to the program are:\n");

	// 2nd solution:
	 for ( counter = 0; argv[counter] != NULL; counter++)
	  printf("argument %d = %s\n", counter, *(argv+counter) );
	//  printf("argument %d = %s\n", counter, argv[counter] );
	 return 0;
}

