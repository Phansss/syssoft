#include "callbacks.h"

typedef struct write_file {
  sensor_data_t* data; // ! must be present !
  FILE* data_out;
} write_file_t;

int write_file(cb_args_t* cb_args) {
    fprintf(((write_file_t*)cb_args)->data_out,
              "%hd %lf %ld\n",
              ((write_file_t*)cb_args)->data->id,
              ((write_file_t*)cb_args)->data->value, 
              ((write_file_t*)cb_args)->data->ts);
    //printf("Written data: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
    //        ((write_file_t*)cb_args)->data->id, 
    //        ((write_file_t*)cb_args)->data->value,
    //        (long int) ((write_file_t*)attr)->data->ts);
    return DATA_PROCESS_SUCCESS;  
}

int write_file_init(write_file_t** attr) {
    (*attr)->data = NULL;
    (*attr)->data_out = fopen("sensor_data_recv", "w+");
}

int write_file_cleanup(write_file_t** attr) {
    (*attr)->data = NULL;
    (*attr)->data_out = fclose("sensor_data_recv");
}