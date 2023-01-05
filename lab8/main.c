#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <pthread.h>
#include <semaphore.h>
#define DEBUG
#define DEB_EXIT
#include "errmacros.h"





//static pthread_rwlock_t rwlock  = PTHREAD_RWLOCK_INITIALIZER;

FILE* binary_file_in;
FILE* binary_file_out;
sem_t semaphore;
pthread_mutex_t mutex;
pthread_barrier_t barrier;
pthread_barrier_t read_sync_finalize_1;
pthread_barrier_t read_sync_finalize_2;
pthread_rwlock_t rwlock;
pthread_mutex_t write_bin_out;
pthread_cond_t writing_stopped;
pthread_rwlock_t sbuffered_cleanup;
sbuffer_t* buffer = NULL;


extern pthread_t writer;


int main() {

  binary_file_in = fopen("sensor_data", "r");
  binary_file_out = fopen("sensor_data_recv", "w+");
  
  if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) exit(1);


 

  
  thread_attr_t writer_attr = {.binary_file = binary_file_in, .sbuffer = buffer}; //Resources for writer thread

  if(pthread_mutex_init(&mutex, NULL) == -1) DEBUG_PRINTF("Pthread mutext init error\n");
  if(pthread_mutex_init(&write_bin_out, NULL) == -1) DEBUG_PRINTF("Pthread mutext init error\n");
  if(pthread_rwlock_init(&sbuffered_cleanup, NULL) == -1) DEBUG_PRINTF("Pthread rwlock init error\n");
  if(pthread_barrier_init(&barrier, NULL, 2 + NO_OF_READERS) == -1) DEBUG_PRINTF("Pthread barrier init error\n"); // init barrier for 2 wait calls
  if(pthread_barrier_init(&read_sync_finalize_1, NULL, NO_OF_READERS) == -1) DEBUG_PRINTF("Pthread barrier init error\n"); // init barrier for 2 wait calls
  if(pthread_barrier_init(&read_sync_finalize_2, NULL, NO_OF_READERS) == -1) DEBUG_PRINTF("Pthread barrier init error\n"); // init barrier for 2 wait calls

  if(pthread_rwlock_init(&rwlock, NULL) == -1) DEBUG_PRINTF("Pthread rwlock init error\n"); //init rwlock
  if (pthread_cond_init(&writing_stopped, NULL) == -1) DEBUG_PRINTF("Pthread cond init error\n"); //innit cond

  PTH_create(&writer, NULL, writer_thread, &writer_attr); // start writing to sbuffer
  //sem_getvalue(&semaphore, &no_sbuffers);


                                         //  NO_OF_READERS is predefined and sem_getvalue is MT-safe.

                                            
  
  //PTH_create(&reader2, NULL, read, &reader_attr);
  
  pthread_barrier_wait(&barrier);
  


  pthread_barrier_destroy(&barrier);
  fclose(binary_file_in);
  fclose(binary_file_out);
  sbuffer_free(&buffer);
  printf("SUCCESFULLY EXITED MAIN\n");
  
  return 0;
}