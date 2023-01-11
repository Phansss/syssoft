#include "callbacks.h"
#include "datamgr.h"


typedef struct datamgr_process {
    sensor_data_t* data;
} datamgr_process_t;

extern dplist_t* sensor_list;

int datamgr_process(cb_args_t* data_arg) {
    process_sensor_data(((datamgr_process_t*)data_arg));
}

int datamgr_init(datamgr_process_t** attr) {
    (*attr)->data = NULL;
    sensor_list = dpl_create(element_copy, element_free, element_compare, element_print);
    FILE* f_sensor_map = fopen("room_sensor.map", "r");
    parse_sensor_map(f_sensor_map, sensor_list);
    f_sensor_map = fclose("room_sensor.map");
}

int datamgr_cleanup(datamgr_process_t** attr) {
    datamgr_free();
}
