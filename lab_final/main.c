#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "config.h"


#include <stdlib.h>
#include <stdio.h>
#include "debugger.h"
#include "errhandler.h"

#include <pthread.h>
#include <semaphore.h>
#include <inttypes.h>
#include "sbuffer.h"//lab 8
#include "connmgr.h" //lab7
#include "sensor_db.h" //lab6
#include "datamgr.h" //lab 5



#include "errmacros.h"
#include "lib/dplist.h"
#include "lib/tcpsock.h"
#include "main_debug.h"






/*___________________Sbuffer Writers -> read from resources_______________ */
typedef struct connmgr_args {
    data_buffer_t* sbuff_data_buffer;
    my_connection_t* connection;
} connmgr_attr_t;
int connmgr_poll(cb_args_t* data_arg);
int connmgr_init(connmgr_attr_t**attr);
int connmgr_cleanup(connmgr_attr_t** attr);

typedef struct read_file {
    data_buffer_t* sbuff_data_buffer; // !must be present !
    FILE* data_in;
} read_file_t;
int read_file(cb_args_t* data_arg);
int read_file_init(read_file_t** attr);
int read_file_cleanup(read_file_t** attr);

/*___________________Sbuffer Readers -> write to resources_______________ */
typedef struct write_file_args {
  data_buffer_t* sbuff_data_buffer; // ! must be present !
  FILE* data_out;
} write_file_t;
int write_file(cb_args_t* data_arg);
int write_file_init(write_file_t** attr);
int write_file_cleanup(write_file_t** attr);


typedef struct dbinsert_args {
    data_buffer_t* sbuff_data_buffer; // ! must be present !
    DBCONN *conn;
} dbinsert_t;
int dbinsert(cb_args_t*data_arg);
int dbinsert_init(dbinsert_t** attr, char clear_up_flag);
int dbinsert_cleanup(dbinsert_t** attr);

typedef struct datamgr_process_args {
    data_buffer_t* sbuff_data_buffer; // ! must be present !
} datamgr_process_t;
int datamgr_process(cb_args_t* data_arg);
int datamgr_init(datamgr_process_t** attr);
int datamgr_cleanup(datamgr_process_t** attr);


dplist_t* sensor_room_list;
int port_number = PORT;

int main() {
  //int child = fork();
  //SYSCALL_ERROR(child);
  //if (child == 0) {
    
  //} 
  //else {
    pthread_t writer;
    pthread_t reader1;
    pthread_t reader2;
    sbuffer_t* buffer;
    if (sbuffer_init(&buffer, SBUFFER_READER_THREADS, SBUFFER_WRITER_THREADS) != SBUFFER_SUCCESS) exit(1);

    //application
    connmgr_attr_t* connmgr_args;
    connmgr_init(&connmgr_args);
    dbinsert_t* dbinsert_args;
    dbinsert_init(&dbinsert_args, 1);
    datamgr_process_t* datamgr_process_args;
    datamgr_init(&datamgr_process_args);

    //test resources
    write_file_t* write_file_args1;
    write_file_init(&write_file_args1);
    write_file_t* write_file_args2;
    write_file_init(&write_file_args2);
    read_file_t* read_file_args;
    read_file_init(&read_file_args);

    PRINTF_MAIN("Adding callback functions to the sbuffer");
    sbuffer_add_callback(buffer, connmgr_poll, (cb_args_t*)connmgr_args, SBUFFER_WRITER);
    sbuffer_add_callback(buffer, dbinsert, (cb_args_t*)dbinsert_args, SBUFFER_READER);
    sbuffer_add_callback(buffer, datamgr_process, (cb_args_t*)datamgr_process_args, SBUFFER_READER);
    //sleep(1);
    //exit(1);
    PRINTF_MAIN("Initializing Threads");
    PTH_create(&writer, NULL, writer_thread, buffer); 
    PTH_create(&reader1, NULL, reader_thread, buffer); 
    PTH_create(&reader2, NULL, reader_thread, buffer); 
    
    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    // Close resources
    connmgr_cleanup(&connmgr_args);
    dbinsert_cleanup(&dbinsert_args);
    datamgr_cleanup(&datamgr_process_args);
    read_file_cleanup(&read_file_args);
    write_file_cleanup(&write_file_args1);
    write_file_cleanup(&write_file_args2);
  
    sbuffer_free(&buffer);
  //}  
  //return 0;
}

int datamgr_process(cb_args_t* data_arg) {
    PRINTF_MAIN("Passing data from sbuffer to datamgr: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld", 
                                        ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->id, 
                                        ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->value, 
                                        ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->ts);
    process_sensor_data(((datamgr_process_t*)data_arg)->sbuff_data_buffer->head);
    return DATA_PROCESS_SUCCESS;
}

int datamgr_init(datamgr_process_t** attr) {
    PRINTF_MAIN("Creating dplist");
    *attr = calloc(1, sizeof(datamgr_process_t));
    ERROR_IF(*attr == NULL, ERR_MALLOC("datamgr_process_t"));
    sensor_room_list = dpl_create(my_sensor_copy, my_sensor_free, my_sensor_compare, my_sensor_print);
    FILE* f_sensor_map = fopen("room_sensor.map", "r");
    PRINTF_MAIN("Parsing sensor map...");
    parse_sensor_map(f_sensor_map, &sensor_room_list);
    fclose(f_sensor_map);
    
    return 0;
}

int datamgr_cleanup(datamgr_process_t** attr) {
    datamgr_free();
    (*attr) = NULL;
    return 0;
}


int dbinsert(cb_args_t* data_arg) {
    insert_sensor(((dbinsert_t*)data_arg)->conn, 
                    ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->id,
                    ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->value,
                    ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->ts);
    PRINTF_MAIN("Passing data from sbuffer to dbinsert: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld", 
                                        ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->id, 
                                        ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->value, 
                                        ((dbinsert_t*)data_arg)->sbuff_data_buffer->head->ts);
    return DATA_PROCESS_SUCCESS;
}

int dbinsert_init(dbinsert_t** attr, char clear_up_flag) {
    (*attr) = (dbinsert_t*) calloc(1, sizeof(dbinsert_t));
    ERROR_IF((*attr) == NULL, ERR_MALLOC("dbinsert_t"));
    (*attr)->conn = init_connection(clear_up_flag);
    printf("Initiating connection...\n");
    return 0;
}

int dbinsert_cleanup(dbinsert_t** attr) {
    disconnect((*attr)->conn);
    (*attr)->sbuff_data_buffer = NULL;
    free(*attr);
    (*attr) = NULL;
    return 0;
}

int connmgr_poll(cb_args_t* data_arg) {       
    connmgr_rx_data_t* rx_data_array = NULL;
    data_arg->data_buffer->count_received = 0;
    if(myconn_listen(((connmgr_attr_t*)data_arg)->connection) == CONNMGR_CLOSE) return DATA_PROCESS_ERROR;
    myconn_get_rx_data(((connmgr_attr_t*)data_arg)->connection, &rx_data_array);
    if(rx_data_array->buff_size > ((connmgr_attr_t*)data_arg)->sbuff_data_buffer->size) {
        //TO DO: write function in sbuffer.h to resize callback function buffer. 
    }
    int thread_rx_buffer_idx = 0;
    for (int i = 0; i < rx_data_array->buff_size; i++) {
        if((rx_data_array->pollrx_buffer_flag)[i] == true) {
            //printf("sensor id = %" PRIu16 " - temperature = %lf - timestamp = %ld\n", (&((rx_data_array->pollrx_buffer)[i]))->id, 
            //                                                                         (&((rx_data_array->pollrx_buffer)[i]))->value, 
            //                                                                         (&((rx_data_array->pollrx_buffer)[i]))->ts);
            (data_arg->data_buffer->head)[thread_rx_buffer_idx].id = (&((rx_data_array->pollrx_buffer)[i]))->id;
            (data_arg->data_buffer->head)[thread_rx_buffer_idx].value = (&((rx_data_array->pollrx_buffer)[i]))->value;
            (data_arg->data_buffer->head)[thread_rx_buffer_idx].ts = (&((rx_data_array->pollrx_buffer)[i]))->ts;
            thread_rx_buffer_idx++;
        }
    }
    data_arg->data_buffer->count_received = thread_rx_buffer_idx;
    PRINTF_MAIN("Total received: %d", data_arg->data_buffer->count_received);
    return DATA_PROCESS_SUCCESS;        
}

int connmgr_init(connmgr_attr_t** attr) {
    (*attr) = (connmgr_attr_t*) calloc(1, sizeof(connmgr_attr_t));
    ERROR_IF((*attr) == NULL, ERR_MALLOC("connmgr_attr_t"));
    (*attr)->sbuff_data_buffer = NULL; //sbuffer takes care of this
    myconn_init(&((*attr)->connection), port_number);
    return 0;
}

int connmgr_cleanup(connmgr_attr_t** attr){
    myconn_destroy(&(*attr)->connection);
    (*attr)->sbuff_data_buffer = NULL;
    return 0;
}


#define _sbuff_data_buffer_head_ ((read_file_t*)data_arg)->sbuff_data_buffer->head
int read_file(cb_args_t* data_arg) {
    if (fread(&(((_sbuff_data_buffer_head_)[0]).id), sizeof(sensor_id_t), 1, 
        ((read_file_t*)data_arg)->data_in) != 1){ ((read_file_t*)data_arg)->sbuff_data_buffer->count_received = 0; return DATA_PROCESS_ERROR;}
    if (fread(&(((_sbuff_data_buffer_head_)[0]).value), sizeof(sensor_value_t), 1, 
        ((read_file_t*)data_arg)->data_in) != 1){ ((read_file_t*)data_arg)->sbuff_data_buffer->count_received = 0; return DATA_PROCESS_ERROR;}
    if (fread(&(((_sbuff_data_buffer_head_)[0]).ts), sizeof(sensor_ts_t), 1, 
        ((read_file_t*)data_arg)->data_in) != 1){ ((read_file_t*)data_arg)->sbuff_data_buffer->count_received = 0; return DATA_PROCESS_ERROR;}
    ((read_file_t*)data_arg)->sbuff_data_buffer->count_received = 1;

    //PRINTF_MAIN("Received data: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld",
    //        (_sbuff_data_buffer_head_)->id, 
    //        (_sbuff_data_buffer_head_)->value,
    //        (long int) (_sbuff_data_buffer_head_)->ts);
    return DATA_PROCESS_SUCCESS;    
}

int read_file_init(read_file_t** attr) {
    (*attr) = (read_file_t*) calloc(1, sizeof(read_file_t));
    ERROR_IF((*attr) == NULL, ERR_MALLOC("read_file_t"));
    (*attr)->sbuff_data_buffer = NULL;
    (*attr)->data_in = fopen("sensor_data", "r");
    //PRINTF_MAIN("initialized read_file init with data_buffer=%p and File pointer=%p", ((*attr)->sbuff_data_buffer), ((*attr)->data_in));
    return 0;
}

int read_file_cleanup(read_file_t** attr) {
    fclose((*attr)->data_in);
    (*attr)->sbuff_data_buffer = NULL;
    //free(*attr);
    (*attr) = NULL;
    return 0;
}


int write_file(cb_args_t* cb_args) {
    //PRINTF_MAIN("Entered write_file callback");
    fprintf(((write_file_t*)cb_args)->data_out,
              "%hd %lf %ld\n",
              (((write_file_t*)cb_args)->sbuff_data_buffer->head)->id,
              (((write_file_t*)cb_args)->sbuff_data_buffer->head)->value, 
              (((write_file_t*)cb_args)->sbuff_data_buffer->head)->ts);
    //printf("Written data: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
    //        ((write_file_t*)cb_args)->data->id, 
    //        ((write_file_t*)cb_args)->data->value,
    //        (long int) ((write_file_t*)attr)->data->ts);
    return DATA_PROCESS_SUCCESS;  
}

int write_file_init(write_file_t** attr) {
    (*attr) = (write_file_t*) calloc(1, sizeof(write_file_t));
    ERROR_IF((*attr) == NULL, ERR_MALLOC("write_file_t"));
    (*attr)->sbuff_data_buffer = NULL;
    (*attr)->data_out = fopen("sensor_data_recv", "w+");
    return 0;
}

int write_file_cleanup(write_file_t** attr) {
    fclose((*attr)->data_out);
    (*attr)->sbuff_data_buffer = NULL;
    //free(*attr);
    (*attr) = NULL;
    return 0;
}