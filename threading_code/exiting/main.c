#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define LOOPS 5

void* f( void* threadID) {

  // this code runs in a thread
    
  int runLoops = LOOPS;  
  int id = *(int *)threadID;
  int *result = malloc( sizeof(int) );
  // check if malloc returned NULL ...
  
  while ( runLoops-- ) {
    printf("Thread %d is working!\n", id);
    sleep(2);
  } 
  
  *result = 555;
  pthread_exit( (void*)result );
}


int main(void) 
{
  int runLoops = LOOPS; 
  pthread_t threadHandle;
  int id = 1234;
  int *result;

  pthread_create( &threadHandle, NULL, &f, (void *)&id );

  while ( runLoops-- ) {
    printf("Main thread is also working!\n");
    sleep(1);
  } 

  pthread_join( threadHandle, (void **)&result );
  
  printf("Thread exited with return value %d.\n", *result);
  free( result );
 
  pthread_exit(NULL);  
}
