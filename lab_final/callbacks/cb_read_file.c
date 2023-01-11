#include "callbacks.h"

typedef struct read_file {
    sensor_data_t* data; // ! must be present !
    FILE* data_in;
} read_file_t;

int read_file(cb_args_t* data_arg) {
    if (fread(&(((read_file_t*)data_arg)->data->id), sizeof(sensor_id_t), 1, 
        ((read_file_t*)data_arg)->data_in) != 1) return DATA_PROCESS_ERROR;
    if (fread(&(((read_file_t*)data_arg)->data->value), sizeof(sensor_value_t), 1, 
        ((read_file_t*)data_arg)->data_in) != 1) return DATA_PROCESS_ERROR;
    if (fread(&(((read_file_t*)data_arg)->data->ts), sizeof(sensor_ts_t), 1, 
        ((read_file_t*)data_arg)->data_in) != 1) return DATA_PROCESS_ERROR;
    //printf("Received data: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
    //        ((read_file_t*)data_arg)->data->id, 
    //        ((read_file_t*)data_arg)->data->value,
    //        (long int) ((read_file_t*)data_arg)->data->ts);
    return DATA_PROCESS_SUCCESS;    
}

int read_file_init(read_file_t** attr) {
    (*attr)->data = NULL;
    (*attr)->data_in = fopen("sensor_data", "r");
}

int read_file_cleanup(read_file_t** attr) {
    (*attr)->data = NULL;
    (*attr)->data_in = fclose("sensor_data");
}