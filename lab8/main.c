#define _GNU_SOURCE


#include "config.h"
#include "sbuffer.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <inttypes.h>
#define DEBUG
#define DEB_EXIT
#include "errmacros.h"
#include "lib/dplist.h"


#include "main_debug.h"


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

FILE* binary_file_in;
FILE* binary_file_out;

pthread_t writer;
pthread_t reader1;
pthread_t reader2;
sbuffer_t* buffer;


int rd_write_to_file(cb_args_t* data_arg) {
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
int wr_read_from_file(cb_args_t* data_arg) {
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
  //initialize sbuffer
  if (sbuffer_init(&buffer, SBUFF_READER_THREADS, SBUFF_WRITER_THREADS) != SBUFFER_SUCCESS) exit(1);
  

  fread_attr_t bin_in_args = { .data = NULL, 
                              .data_in = binary_file_in};
  
  fprintf_attr_t bin_out_args = { .data = NULL,
                                 .data_out = binary_file_out};
   
  sbuffer_add_callback(buffer, wr_read_from_file, (cb_args_t*)&bin_in_args, SBUFFER_WRITER);
  sbuffer_add_callback(buffer, rd_write_to_file, (cb_args_t*)&bin_out_args, SBUFFER_READER);

  //sbuffer_remove_callback(buffer, wr_read_from_file, (cb_args_t*)&bin_in_args);
  //sbuffer_remove_callback(buffer, rd_write_to_file, (cb_args_t*)&bin_out_args);

  printf("read_from_file_func AT %p\n", wr_read_from_file);
  printf("bin out args at %p\n", &bin_in_args);

  printf("write_to_file_func AT %p\n", rd_write_to_file);
  printf("bin out args at %p\n", &bin_out_args);


  printf("buffer pointer in main: %p\n", buffer);
  PTH_create(&writer, NULL, writer_thread, buffer); // start writing to sbuffer
  
  PTH_create(&reader1, NULL, reader_thread, buffer); // Start reading (tries sem and yields immediately (sem ==0))
  PTH_create(&reader2, NULL, reader_thread, buffer); // Start reading (tries sem and yields immediately (sem ==0)) 
  printf("waiting for writer\n");
  pthread_join(writer, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);

  fclose(binary_file_in);
  fclose(binary_file_out);
  sbuffer_free(&buffer);
  
  return 0;
}

