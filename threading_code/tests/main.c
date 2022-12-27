#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define MAX 10

#define A_VALUE  0
#define B_VALUE  -1

// shared array
int data[MAX];


void print_data( char c )
{
	int i;
	printf("\n");
	for ( i = 0 ; i < MAX ; i++ )
	{
		printf("[%c%d]=%d ",c, i, data[i]);
	}
	printf("\n");
}

void *fighter_A( void *id) {
	// this code runs in a thread
	int i;
	while (1)
	{
		// set all values in data to A_VALUE but data is unprotected
		for ( i = 0 ; i < MAX ; i++ )
		{
			data[i] = A_VALUE;
		}
		
		print_data('A');
	}

	pthread_exit( NULL );
}


void *fighter_B( void *id) {
	// this code runs in a thread
	int i;
	while (1)
	{
		// set all values in data to B_VALUE but data is unprotected
		for ( i = 0 ; i < MAX ; i++ )
		{
			data[i] = B_VALUE;
		}
		
		print_data('B');
	}

	pthread_exit( NULL );
}

int main(void) {
	pthread_t thread_A, thread_B;
	
	pthread_create( &thread_A, NULL, &fighter_A, NULL );
	pthread_create( &thread_B, NULL, &fighter_B, NULL );

	pthread_exit(NULL);  
}
