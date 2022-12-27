//#define _POSIX_C_SOURCE
#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX 3

#define A_VALUE  'a'
#define B_VALUE  'b'

// shared array
volatile char data[MAX];


// mutex
pthread_barrier_t sync_here;

void pthread_err_handler( int err_code, char *msg, char *file_name, char line_nr )
{
	if ( 0 != err_code )
	{
		fprintf( stderr, "\n%s failed with error code %d in file %s at line %d\n", msg, err_code, file_name, line_nr );
		//errno = err_code;
		//perror("Error message: ");
	}
}


void * f( void *id) {
	// this code runs in a thread
	while (1)
	{
	  // do some work here ...
	  if (*(int *)id == 1 )
	    sleep(3);
	  else
	    usleep(100);
	  printf("thread %d just before the barrier\n", *(int *)id);
	  // run this code with and without 'pthread_barrier_wait' to see the differenct behavior
	  pthread_barrier_wait( &sync_here );
	  printf("thread %d just after the barrier\n", *(int *)id);
	}

	pthread_exit( NULL );
}


int main(void) {
	int presult;
	pthread_t thread_A, thread_B, thread_C;
	int id_A, id_B, id_C;
	

	presult = pthread_barrier_init( &sync_here, NULL, 3 );
	pthread_err_handler( presult, "pthread_mutex_destroy", __FILE__, __LINE__ );	

	id_A = 1;	
	presult = pthread_create( &thread_A, NULL, &f, &id_A );
	pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
	
	id_B = 2;	
	presult = pthread_create( &thread_B, NULL, &f, &id_B );
	pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
	
	id_C = 3;
	presult = pthread_create( &thread_C, NULL, &f, &id_C );
	pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
	
	// important: don't forget to join, otherwise main thread exists and destroys the mutex
	presult= pthread_join(thread_A, NULL);
	pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );

	presult= pthread_join(thread_B, NULL);
	pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );

	presult= pthread_join(thread_C, NULL);
	pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );	
	
	presult = pthread_barrier_destroy( &sync_here );
	pthread_err_handler( presult, "pthread_mutex_destroy", __FILE__, __LINE__ );

	pthread_exit(NULL);
}
