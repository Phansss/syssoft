#define _GNU_SOURCE



#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <pthread.h>
#include <semaphore.h>
#include <inttypes.h>
#define DEBUG
#define DEB_EXIT
#include "errmacros.h"

#define WRITER_DATA_IN_EMPTY -1

typedef size_t (*Freadfunc)(void*, size_t, size_t, FILE*);

//Writer uses fread as func
typedef struct fread_attr {
    sensor_data_t* data; //IMPORTANT FOR USERS OF SBUFFER THREADS: -> FIRST DEFINE DATA IN THE DATA PROCESSING ATTRIBUTES
    FILE* data_in;
} fread_attr_t;

typedef struct fprintf_attr {
  sensor_data_t* data;
  FILE* data_out;
} fprintf_attr_t;


//static pthread_rwlock_t rwlock  = PTHREAD_RWLOCK_INITIALIZER;

FILE* binary_file_in;
FILE* binary_file_out;

extern pthread_barrier_t sbuff_reader_sync;
extern pthread_barrier_t sbuff_start_readers;
extern pthread_mutex_t sbuff_reader_removing;
extern pthread_mutex_t sbuff_reader_processing;
extern pthread_rwlock_t sbuff_writer_inserting;
extern pthread_cond_t sbuff_writer_buffered;
extern sem_t sbuff_node_count;
extern sbuffer_t* buffer;
extern pthread_t writer;
extern pthread_t reader1;
extern pthread_t reader2;


int write_to_file(dataprocessor_arg_t* data_arg) {
    fprintf(((fprintf_attr_t*)data_arg)->data_out,
              "%hd %lf %ld\n",
              ((fprintf_attr_t*)data_arg)->data->id,
              ((fprintf_attr_t*)data_arg)->data->value, 
              ((fprintf_attr_t*)data_arg)->data->ts);
    //printf("Written data: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
    //        ((fprintf_attr_t*)data_arg)->data->id, 
    //        ((fprintf_attr_t*)data_arg)->data->value,
    //        (long int) ((fprintf_attr_t*)data_arg)->data->ts);
    return DATA_PROCESS_SUCCESS;  
}

// Data processing function that reads in data from a binary file (writer thread).
int read_from_file(dataprocessor_arg_t* data_arg) {
    if (fread(&(((fread_attr_t*)data_arg)->data->id), sizeof(sensor_id_t), 1, 
        ((fread_attr_t*)data_arg)->data_in) != 1) return DATA_PROCESS_ERROR; 
    if (fread(&(((fread_attr_t*)data_arg)->data->value), sizeof(sensor_value_t), 1, 
        ((fread_attr_t*)data_arg)->data_in) != 1) return DATA_PROCESS_ERROR;
    if (fread(&(((fread_attr_t*)data_arg)->data->ts), sizeof(sensor_ts_t), 1, 
        ((fread_attr_t*)data_arg)->data_in) != 1) return DATA_PROCESS_ERROR;

    //printf("Received data: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
    //        ((fread_attr_t*)data_arg)->data->id, 
    //        ((fread_attr_t*)data_arg)->data->value,
    //        (long int) ((fread_attr_t*)data_arg)->data->ts);
    return DATA_PROCESS_SUCCESS;    
}

int main() {

  binary_file_in = fopen("sensor_data", "r");
  binary_file_out = fopen("sensor_data_recv", "w+");

  printf("MAIN: opened file pointer %p\n", binary_file_in);

  //initialize sbuffer
  if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) exit(1);

  fread_attr_t fread_arguments = {.data = NULL, 
                                  .data_in = binary_file_in};
  fprintf_attr_t fprintf_arguments1 = { .data = NULL,
                                       .data_out = binary_file_out};
  fprintf_attr_t fprintf_arguments2 = { .data = NULL,
                                       .data_out = binary_file_out};
 
  sbuff_thread_arg_t writer1_arg = { .sbuffer = buffer, 
                                     .data_processor = read_from_file, 
                                     .data_processor_arguments = (dataprocessor_arg_t*)&fread_arguments};

  sbuff_thread_arg_t reader1_arg = { .sbuffer = buffer, 
                                     .data_processor = write_to_file, 
                                     .data_processor_arguments = (dataprocessor_arg_t*)&fprintf_arguments1};

  sbuff_thread_arg_t reader2_arg = { .sbuffer = buffer, 
                                     .data_processor = write_to_file, 
                                     .data_processor_arguments = (dataprocessor_arg_t*)&fprintf_arguments2};
 


  //initialize barriers
  if(pthread_barrier_init(&sbuff_reader_sync, NULL, SBUFF_READER_THREADS) == -1) DEBUG_PRINTF("Pthread barrier init error\n"); // init barrier for 2 wait calls
  if(pthread_barrier_init(&sbuff_start_readers, NULL, SBUFF_READER_THREADS + SBUFF_WRITER_THREADS) == -1) DEBUG_PRINTF("Pthread barrier init error\n"); // init barrier for 2 wait calls
  //used in readers:
  if(sem_init(&sbuff_node_count, 0, 0) == -1) DEBUG_PRINTF("Pthread semaphore init error\n"); // create semaphore

  //create writer thread
  PTH_create(&writer, NULL, writer_thread, &writer1_arg); // start writing to sbuffer
  PTH_create(&reader1, NULL, reader_thread, &reader1_arg); // Start reading (tries sem and yields immediately (sem ==0))
  PTH_create(&reader2, NULL, reader_thread, &reader2_arg); // Start reading (tries sem and yields immediately (sem ==0))
  
  pthread_join(writer, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);
                                         //  SBUFF_READER_THREADS is predefined and sem_getvalue is MT-safe.

                                            
  
  //PTH_create(&reader2, NULL, read, &reader_attr);
  
  //pthread_barrier_wait(&sbuff_main_rw_sync);
 

  pthread_barrier_destroy(&sbuff_reader_sync);
  pthread_rwlock_destroy(&sbuff_writer_inserting);

  fclose(binary_file_in);
  fclose(binary_file_out);
  sbuffer_free(&buffer);
  printf("SUCCESFULLY EXITED MAIN\n");
  
  return 0;
}