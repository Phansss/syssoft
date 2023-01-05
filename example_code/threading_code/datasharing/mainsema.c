#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

#define MAX 10

#define THREAD_SHARING 0  // semaphore is shared between threads of the same process and not between processes - no shared memory needs to be created to store the semaphore
#define SEMA_INIT_VALUE 2

// shared array
volatile char shared_data[MAX];

// semaphore
sem_t data_sema;

void err_handler( int err_code, char *msg, char *file_name, char line_nr )
{
	if ( 0 != err_code )
	{
		fprintf( stderr, "\n%s failed with error code %d in file %s at line %d\n", msg, err_code, file_name, line_nr );
		//errno = err_code;
		//perror("Error message: ");
	}
}

                
void *fighter_A( void *id) {
  // this code runs in a thread
  int i, presult;
  while (1)
  {
    presult = sem_wait( &data_sema );
    err_handler( presult, "sem_wait", __FILE__, __LINE__ );
    // make reading and writing to shared buffer atomic
    for ( i = 0 ; i < MAX ; i++ )
    {
      shared_data[i] = 'a';

    }
    printf("\nFighter A is printing all values:\n");
    // print all values in shared_data
    for ( i = 0 ; i < MAX ; i++ )
    {
        //usleep(500);  // 'help' a context switch here
        printf("[%d]=%c ", i, shared_data[i]);
    }
    presult = sem_post( &data_sema );
    err_handler( presult, "sem_post", __FILE__, __LINE__ );
  }
  
  pthread_exit( NULL );
}


void *fighter_B( void *id) {
  // this code runs in a thread
  int i, presult;
  while (1)
  {
    presult = sem_wait( &data_sema );
    err_handler( presult, "sem_wait", __FILE__, __LINE__ );
    // make reading and writing to shared buffer atomic
    for ( i = 0 ; i < MAX ; i++ )
    {
      shared_data[i] = 'b';

    }
    printf("\nFighter B is printing all values: \n");
    // print all values in shared_data
    for ( i = 0 ; i < MAX ; i++ )
    {
        //usleep(500);  // 'help' a context switch here
        printf("[%d]=%c ", i, shared_data[i]);
    }
    presult = sem_post( &data_sema );
    err_handler( presult, "sem_post", __FILE__, __LINE__ );
  }
  
  pthread_exit( NULL );
}
                
                
                
int main(void) {
	int presult;
	pthread_t thread_A, thread_B;
	
	presult = sem_init( &data_sema, THREAD_SHARING, SEMA_INIT_VALUE);
	err_handler( presult, "sem_init", __FILE__, __LINE__ );
	
	presult = pthread_create( &thread_A, NULL, &fighter_A, NULL );
	err_handler( presult, "pthread_create", __FILE__, __LINE__ );
	
	presult = pthread_create( &thread_B, NULL, &fighter_B, NULL );
	err_handler( presult, "pthread_create", __FILE__, __LINE__ );

	// important: don't forget to join, otherwise main thread exists and destroys the mutex
	presult= pthread_join(thread_A, NULL);
	err_handler( presult, "pthread_join", __FILE__, __LINE__ );

	presult= pthread_join(thread_B, NULL);
	err_handler( presult, "pthread_join", __FILE__, __LINE__ );

	presult = sem_destroy( &data_sema );
	err_handler( presult, "sem_destroy", __FILE__, __LINE__ );

	pthread_exit(NULL);
}
