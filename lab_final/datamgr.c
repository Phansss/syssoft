#define _GNU_SOURCE

#include "config.h"
#include "errhandler.h"
#include "debugger.h"
#include "datamgr.h"
#include "lib/dplist.h"
#include <assert.h>
#include <inttypes.h>

#define DATAMGR_READ_ERROR 1
/** Structure to store sensor logistics.
 * 
*/
typedef struct my_sensor {
    uint16_t rid;                             // room id
    sensor_id_t sid;                          // sensor id
    sensor_value_t ravg;                      // running average
    sensor_value_t rvalues[RUN_AVG_LENGTH];   // running values
    sensor_ts_t lm;                           // last modified
    int rvalues_valid;                        // count non-zero values in rvalues at startup of the application
} my_sensor_t;


//Wrapper function for lab 5. Not used in final application.
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    // Create and parse sensors into dplist
    sensor_room_list = dpl_create(my_sensor_copy, my_sensor_free, my_sensor_compare, my_sensor_print);
    parse_sensor_map(fp_sensor_map, &sensor_room_list);

    sensor_data_t data_rx = {.id = 0, .ts = 0, .value = 0};
    while  (fread(&(data_rx.id), sizeof(sensor_id_t), 1, fp_sensor_data) != 0) {
        fread(&(data_rx.value), sizeof(sensor_value_t), 1, fp_sensor_data);
        fread(&(data_rx.ts), sizeof(sensor_ts_t), 1, fp_sensor_data); 
        process_sensor_data(&data_rx);
    }
}

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free() {
    dpl_free(&sensor_room_list, 1);
}


void parse_sensor_map(FILE *fp_sensor_map, dplist_t** dplist){
    my_sensor_t* new_sensor = NULL;
    sensor_create(&new_sensor, 0, 0);
    while(fscanf(fp_sensor_map, "%h"PRIu16 " %h"PRIu16, &((new_sensor)->rid), &((new_sensor)->sid)) != EOF) {
        dpl_insert_sorted(*dplist, new_sensor, true);
    }
    free(new_sensor);
}

void process_sensor_data(sensor_data_t* data) {
    my_sensor_t* curr_sensor;                
 
    //Search for the sensor in dplist
    curr_sensor = sensor_search(sensor_room_list, data->id);
    if (curr_sensor == NULL) { 
        PRINTF_DATAMGR("Received data for missing sensor with sid=%d !", data->id);
        return;
    }
    
    //Update the sensor in the dplist 
    sensor_update(curr_sensor, data->value, data->ts);

    //Print a message depending on the running average
    char buff[25];
    struct tm * timeinfo;
    timeinfo = localtime(&(curr_sensor->lm));
    strftime(buff, sizeof(buff), "%c", timeinfo);

    if (curr_sensor->ravg > SET_MAX_TEMP) fprintf(stderr, "%s - %2.2lf°C too high for sensor %d in room %d!\n", 
        buff, curr_sensor->ravg, curr_sensor->sid, curr_sensor->rid);
    if (curr_sensor->ravg < SET_MIN_TEMP) fprintf(stderr, "%s - %2.2lf°C too low for sensor %d in room %d!\n", 
        buff, curr_sensor->ravg, curr_sensor->sid, curr_sensor->rid);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    my_sensor_t* sensor = sensor_search(sensor_room_list, sensor_id);
    //ERROR_HANDLER(sensor == NULL, ERR_INVALID_ID);
    return sensor->rid;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    my_sensor_t* sensor = sensor_search(sensor_room_list, sensor_id);
    //ERROR_HANDLER(sensor == NULL, ERR_INVALID_ID);
    return sensor->ravg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    my_sensor_t* sensor = sensor_search(sensor_room_list, sensor_id);
    //ERROR_HANDLER(sensor == NULL, ERR_INVALID_ID);
    return sensor->lm;
}

// TO DO: Check for duplicate IDs in different rooms(?)
int datamgr_get_total_sensors() {
    return dpl_size(sensor_room_list);
}


/**********************************************************_HELPER FUNCTIONS_*********************************************************************/
int sensor_create(my_sensor_t** sensor, sensor_id_t sid, uint16_t rid) {
    ERROR_IF((*sensor) != NULL, "Sensor pointer not null!"); if ((*sensor) != NULL) {return DATAMGR_FAILURE;};
    *sensor = malloc(sizeof(my_sensor_t));
    ERROR_IF((*sensor) == NULL, ERR_MALLOC("my_sensor_t")); if (*sensor==NULL) {return DATAMGR_FAILURE;};
    (*sensor)->sid = sid;
    (*sensor)->rid = rid;  
    (*sensor)->ravg = 0;
    //time(&((*sensor)->lm)); //initialize last modified to current time
    (*sensor)->lm = 0;
    (*sensor)->rvalues_valid = 0;
    return DATAMGR_SUCCESS;
;}

my_sensor_t* sensor_search(dplist_t* sensor_dplist, sensor_id_t sid) {
    ERROR_IF(sensor_dplist == NULL, "Invalid function argument"); if (sensor_dplist==NULL) {return NULL;};
    my_sensor_t dummy;
    dummy.sid = sid; 
    int idx = dpl_get_index_of_element(sensor_dplist, &dummy);
    if (idx==-1) {PRINTF_DATAMGR("sensor with sid not found in the list!"); return NULL;}
    my_sensor_t* sensor = dpl_get_element_at_index(sensor_dplist, idx);
    ERROR_IF(sensor == NULL, "sensor with idx %d not found in the list!", idx); if (sensor==NULL) return NULL;
    return sensor;     
}

int sensor_update(my_sensor_t* sensor, sensor_value_t value, sensor_ts_t ts) {
    ERROR_IF(sensor == NULL, "Invalid function argument"); if (sensor==NULL) {return DATAMGR_FAILURE;};
    sensor_push_value(sensor, value);
    sensor_update_ravg(sensor);
    sensor->lm = ts;
    return DATAMGR_SUCCESS;
}

int sensor_print(my_sensor_t* sensor) {
    ERROR_IF(sensor == NULL, "Invalid function argument"); if (sensor==NULL) {return DATAMGR_FAILURE;};
    char buff[25];
    struct tm * timeinfo;
    timeinfo = localtime(&(sensor->lm));
    strftime(buff, sizeof(buff), "%c", timeinfo);

    if (sensor->sid < 100) printf("[%1s%d@%s]:    %2.2lf    ", " ", sensor->sid, buff, sensor->ravg);
    else printf("[%d@%s]:    %2.2lf    ", sensor->sid, buff,sensor->ravg);

    int i=0;
    printf("[");
    for (i=0; i<RUN_AVG_LENGTH-1; i++) {
        printf("%2.2lf, ", (sensor->rvalues)[i]);
    }
    printf("%2.2lf]\n", (sensor->rvalues)[RUN_AVG_LENGTH-1]);
    return DATAMGR_SUCCESS;
}

int sensor_push_value(my_sensor_t* sensor, sensor_value_t value) {
    ERROR_IF(sensor == NULL, "Invalid function argument"); if (sensor==NULL) {return DATAMGR_FAILURE;};
    int i=0;
    for (i=0; i<(RUN_AVG_LENGTH-1); i++) {              //
        (sensor->rvalues)[i] = (sensor->rvalues)[i+1];  // Running right-to-left FIFO buffer
    }                                                   //
    (sensor->rvalues)[RUN_AVG_LENGTH-1] = value;        //
    return DATAMGR_SUCCESS;
}

int sensor_update_ravg(my_sensor_t* sensor) {
    ERROR_IF(sensor == NULL, "Invalid function argument"); if (sensor==NULL) {return DATAMGR_FAILURE;};
    sensor_value_t sum=0;
    int i=0;
    for (i=0; i<(RUN_AVG_LENGTH); i++) { 
        sum = sum + (sensor->rvalues)[i];
    }
    if(sensor->rvalues_valid != RUN_AVG_LENGTH) {
        sensor->rvalues_valid++;
        sensor->ravg = 0;
    }
    else {
        sensor->ravg = sum/RUN_AVG_LENGTH;
    }
    return DATAMGR_SUCCESS;
}


/********************************************************* _DPLIST CALLBACKS_*******************************************************************/
void * my_sensor_copy(void * element) {
    my_sensor_t* copy = malloc(sizeof (my_sensor_t));
    assert(copy != NULL);
    copy->rid = ((my_sensor_t*)element)->rid;
    copy->sid = ((my_sensor_t*)element)->sid;
    copy->ravg = ((my_sensor_t*)element)->ravg;
    copy->lm = ((my_sensor_t*)element)->lm; //timestamp last modified
    copy->rvalues_valid = ((my_sensor_t*)element)->rvalues_valid; //valid running values
    for (int i=0; i < RUN_AVG_LENGTH; i++) {
        (copy->rvalues)[i] = (((my_sensor_t*)element)->rvalues)[i];
    }
    return (void *) copy;
}

void my_sensor_free(void ** element) {
    if (*element == NULL) return;
    else {
        free(*element);
        *element = NULL;
    }
}

int my_sensor_compare(void * x, void * y) {
    return ((((my_sensor_t*)x)->sid < ((my_sensor_t*)y)->sid) ? -1 : (((my_sensor_t*)x)->sid == ((my_sensor_t*)y)->sid) ? 0 : 1);
}

void my_sensor_print(void* element) {
    char buff[25];
    struct tm* timeinfo;
    timeinfo = localtime(&(((my_sensor_t*)element)->lm));
    strftime(buff, 25, "%c", timeinfo);
    printf(" (rid$=%d, sid=%d, ravg = %1.2f, ts = %s\n",  
        ((my_sensor_t*)element)->rid, ((my_sensor_t*)element)->sid, ((my_sensor_t*)element)->ravg, buff);
}






