#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define LOOPS 5

void *f( void *threadID) {

  // this code runs in a thread
    
  int runLoops = LOOPS;  
  int id = *(int *)threadID;
  int *result = malloc( sizeof(int) );
  // check if malloc returned NULL ...
  
  while ( runLoops-- ) {
    printf("Thread %d is working!\n", id);
    sleep(1);
  } 
  
  *result = 555;
  pthread_exit( (void*)result );
}


int main(void) 
{
  int runLoops = LOOPS; 
  pthread_t threadHandle1, threadHandle2;
  int id;
  //int id1, id2; 
  int *result;

  // INCORRECT PARAMETER PASSING EXAMPLE !!!!!!

  id = 1234;	
  pthread_create( &threadHandle1, NULL, &f, (void *)&id );
  
  id = 5678;
  pthread_create( &threadHandle2, NULL, &f, (void *)&id );
  
  // PARAMETER PASSING EXAMPLE: USE ID1&2 or malloc new memory !!!!!!  
  /*
  id1 = 1234;	
  pthread_create( &threadHandle1, NULL, &f, (void *)&id1 );
  
  id2 = 5678;
  pthread_create( &threadHandle2, NULL, &f, (void *)&id2 );

  // ==> be aware: don't use id1 and id2 !!!
  id1=0;
  id2=0;
  */

  while ( runLoops-- ) {
    printf("Main thread is also working!\n");
    sleep(1);
  } 

  pthread_join( threadHandle1, (void **)&result );
  printf("Thread 1 exited with return value %d.\n", *result);
  free( result );
  
  pthread_join( threadHandle2, (void **)&result );
  printf("Thread 2 exited with return value %d.\n", *result);
  free( result );
 
  pthread_exit(NULL);  
}
