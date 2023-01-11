#include "callbacks.h"
#include "../sensor_db.h"

#define _dbinsert_attr_  

typedef struct dbinsert {
    sensor_data_t* data;
    DBCONN *conn;
} dbinsert_t;

int dbinsert(cb_args_t* data_arg) {
    insert_sensor(((dbinsert_t*)data_arg)->conn, 
                    ((dbinsert_t*)data_arg)->data->id,
                    ((dbinsert_t*)data_arg)->data->value,
                    ((dbinsert_t*)data_arg)->data->ts);
}

int dbinsert_init(dbinsert_t** attr, char clear_up_flag) {
    (*attr)->conn = init_connection(clear_up_flag);
    return DATA_PROCESS_SUCCESS;
}

int dbinsert_cleanup(dbinsert_t** attr) {
    disconnect((*attr)->conn);
    return DATA_PROCESS_SUCCESS;
}

