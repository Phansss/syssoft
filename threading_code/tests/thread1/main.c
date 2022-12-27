
// compile as follows: gcc -lm -lpthread main.c 
// -lm : link with math lib
// -lpthread : link with pthread lib

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

// constants
#define MAX 10
#define MAX_COORD 100
#define MAX_SLEEP 5
#define PTHR_CREATE_SUCCESS 0
#define PTHR_JOIN_SUCCESS 0

// typedefs
typedef struct point {
  int x, y;
  } point_t;

typedef struct data {
  int threadNr;
  point_t point;
  float distance;
  } data_t;

// this function will be executed by all threads (= do_something function in slides)
// it computes the distance of a given point (input) to the origin
void *distance(void *a);

// static data (shared between threads)
const int pthr_exit_success = 0;  // need a (const) var for this!
const int pthr_exit_failure = 1;  // need a (const) var for this!
data_t data[MAX];

// main thread
int main(void) {
  int i;
  long int start;
  pthread_t threadHandle[MAX];
  void *result;

  // initialize the data
  start = time( &start );
  srand( start );
  for( i = 0; i < MAX; i++ ) {
    data[i].threadNr = i;
    data[i].point.x = rand() % MAX_COORD;
    data[i].point.y = rand() % MAX_COORD;
  }

  // start up MAX threads - each thread computing the distance of one point
  for( i = 0; i < MAX; i++ ) {
    if( pthread_create( &threadHandle[i], NULL, &distance, (void *)(data+i) ) != PTHR_CREATE_SUCCESS ) { 
      printf("Main thread says:\t couldn't create my threads ... exiting\n");
      exit( EXIT_FAILURE );  // terminates entire process, incl. already created threads
      } 
  } 

  for( i = 0; i < MAX; i++ ) 
    if ( pthread_join( threadHandle[i], &result ) == PTHR_JOIN_SUCCESS ) 
      if ( *(int *)result == pthr_exit_success )
        printf("Main thread says:\t thread %i terminated successfully: distance to origin of point (%d,%d) is %f\n", 
					i, data[i].point.x, data[i].point.y, data[i].distance);
      else if ( *(int *)result == pthr_exit_failure )
        printf("Main thread says:\t thread %i terminated unsuccessfully\n", i );
      else
        printf("Main thread says:\t thread %i terminated with unknown exit code\n", i );
    else
      printf("Main thread says:\t thread_join failed %d\n", i); 

  return 0; 
}


void *distance( void *arg )
{
  data_t *data = (data_t *)arg;

  printf("Thread %d says:\t hello world, I'm alive now ...\n", data->threadNr );

  sleep( rand() % MAX_SLEEP ); // slow down the thread

  data->distance = sqrt( data->point.x * data->point.x + data->point.y * data->point.y );

  sleep( rand() % MAX_SLEEP ); // slow down the thread

  if ( rand() % MAX > data->threadNr ) {  // simulate exit with failure ...
    printf("Thread %d says:\t byebye world, I'm leaving now with exit code \"failure\"\n", data->threadNr );
    pthread_exit( (void *)&pthr_exit_failure ); // need a non-lacal var for this
  }
  else {
    printf("Thread %d says:\t byebye world, I'm leaving now with exit code \"success\"...\n", data->threadNr );
    pthread_exit( (void *)&pthr_exit_success ); // need a non-lacal var for this
  }
}

 
