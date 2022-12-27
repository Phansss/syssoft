#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX 6

#define A_VALUE  'a'
#define B_VALUE  'b'

// shared array
char shared_data[MAX];

void pthread_err_handler( int err_code, char *msg, char *file_name, char line_nr )
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
  int i;
  while (1)
  {
    
    // set all values in shared_data to some value 'a' shared data is unprotected
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
    
  }
  
  pthread_exit( NULL );
}


void *fighter_B( void *id) {
  // this code runs in a thread
  int i;
  while (1)
  {
    
    // set all values in shared_data to some value 'b' shared data is unprotected
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
    
  }
  
  pthread_exit( NULL );
}

int main(void) {
  int presult;
  pthread_t thread_A, thread_B;
  
  presult = pthread_create( &thread_A, NULL, &fighter_A, NULL );
  pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
  
  presult = pthread_create( &thread_B, NULL, &fighter_B, NULL );
  pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
  
  // important: don't forget to join, otherwise main thread exists and destroys process/mutex/...
  presult= pthread_join(thread_A, NULL);
  pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );
  
  presult= pthread_join(thread_B, NULL);
  pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );
  
  pthread_exit(NULL);
}
