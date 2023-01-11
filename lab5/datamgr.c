#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "datamgr.h"
#include "lib/dplist.h"

#include <assert.h>

#ifdef DEBUG
void dpl_print_special(dplist_t* list, char const * caller_name, int caller_line, char const * caller_file )
{
    //printf("----------------------------------------\n");
    printf("\n");
    printf( "%5s Line %d in %s (%s):\n","", caller_line, caller_file, caller_name);
    dpl_print(list);
}
#endif
#ifdef DEBUG
#define dpl_print(list) dpl_print_special(list, __func__, __LINE__, __FILE__)
#endif

dplist_t* sensor_list = NULL;

/*****************************************************LAB METHODS**************************************************************/
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    // Create and parse sensors into dplist
    sensor_list = dpl_create(element_copy, element_free, element_compare, element_print);
    parse_sensor_map(fp_sensor_map, &sensor_list);

    sensor_data_t* buffer = malloc(sizeof(sensor_data_t));            
    assert(buffer != NULL);
    //read and store the data in the sensor list, print logs
    process_sensor_data(fp_sensor_data, sensor_list, buffer);

    free(buffer);
    buffer = NULL;     
}

void datamgr_free() {
    dpl_free(&sensor_list, true);
 }

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    my_sensor_t* sensor = sensor_search(sensor_list, sensor_id);
    ERROR_HANDLER(sensor == NULL, ERR_INVALID_ID);
    return sensor->rid;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    my_sensor_t* sensor = sensor_search(sensor_list, sensor_id);
    ERROR_HANDLER(sensor == NULL, ERR_INVALID_ID);
    return sensor->ravg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    my_sensor_t* sensor = sensor_search(sensor_list, sensor_id);
    ERROR_HANDLER(sensor == NULL, ERR_INVALID_ID);
    return sensor->lm;
}

// TO DO: Check for duplicate IDs in different rooms(?)
int datamgr_get_total_sensors() {
    return dpl_size(sensor_list);
}


/*****************************************************CORE FUNCTIONALITY**************************************************************/
void parse_sensor_map(FILE *fp_sensor_map, dplist_t** dplist){
    my_sensor_t* new_sensor = NULL;
    sensor_create(&new_sensor, 0, 0);
    while(fscanf(fp_sensor_map, "%hd %hd", &((new_sensor)->rid), &((new_sensor)->sid)) != EOF) {
        dpl_insert_sorted(*dplist, new_sensor, true);
    }
    free(new_sensor);
}

void process_sensor_data(FILE *binary_file, dplist_t* sensor_list, sensor_data_t* buffer) {
    my_sensor_t* curr_sensor = NULL;                
    int i = 0;
    while (fread(&(buffer->id), sizeof(sensor_id_t), 1, binary_file)!= 0) {
        i++;
        fread(&(buffer->value), sizeof(sensor_value_t), 1, binary_file);
        fread(&(buffer->ts), sizeof(sensor_ts_t), 1, binary_file);     

        //Search for the sensor in dplist
        curr_sensor = sensor_search(sensor_list, buffer->id);
        if (curr_sensor == NULL) { 
            fprintf(stderr, "Received data for missing sensor (%d)!", buffer->id);
            continue;
        }
        //Update the sensor in the dplist 
        sensor_update(curr_sensor, buffer->value, buffer->ts);
        //sensor_print(curr_sensor);

        char buff[25];
        struct tm * timeinfo;
        timeinfo = localtime(&(curr_sensor->lm));
        strftime(buff, sizeof(buff), "%c", timeinfo);

        if (curr_sensor->ravg > SET_MAX_TEMP) fprintf(stderr, "%s - °C too high for sensor %d in room %d!\n", 
            buff, curr_sensor->ravg, curr_sensor->sid, curr_sensor->rid);
        if (curr_sensor->ravg < SET_MIN_TEMP) fprintf(stderr, "%s - %2.2lf°C too low for sensor %d in room %d!\n", 
            buff, curr_sensor->ravg, curr_sensor->sid, curr_sensor->rid);
    
    }
    //printf("\nSuccessfully read %d data samples.\n\n", i);

}



/**********************************************************SENSOR*********************************************************************/
void sensor_create(my_sensor_t** sensor, sensor_id_t sid, uint16_t rid) {
    assert(*sensor==NULL);
    *sensor = malloc(sizeof(my_sensor_t));
    (*sensor)->sid = sid;
    (*sensor)->rid = rid;  
    (*sensor)->ravg = 0;    //initialize running average to 0
    //time(&((*sensor)->lm)); //initialize last modified to current time
    (*sensor)->lm = 0;      //initialize last modified to earliest possible time.
    (*sensor)->rvalues_valid = 0; 
}

my_sensor_t* sensor_search(dplist_t* sensor_dplist, sensor_id_t sid) {
    my_sensor_t dummy;
    dummy.sid = sid; 
    int idx = dpl_get_index_of_element(sensor_dplist, &dummy);
    if (idx == -1) return NULL;
    my_sensor_t* sensor = dpl_get_element_at_index(sensor_dplist, idx);
    if (sensor != NULL) {
        return sensor;
    }
    return NULL;      
}

my_sensor_t* sensor_update(my_sensor_t* sensor, sensor_value_t value, sensor_ts_t ts) {
    sensor_push_value(sensor, value);
    sensor_update_ravg(sensor);
    sensor->lm = ts;
    return sensor;
}

/* void sensor_print(my_sensor_t* sensor) {
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
}
 */

void sensor_push_value(my_sensor_t* sensor, sensor_value_t value) {
    int i=0;
    for (i=0; i<(RUN_AVG_LENGTH-1); i++) {              //
        (sensor->rvalues)[i] = (sensor->rvalues)[i+1];  // Running right-to-left FIFO buffer
    }                                                   //
    (sensor->rvalues)[RUN_AVG_LENGTH-1] = value;        //
}

void sensor_update_ravg(my_sensor_t* sensor) {
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
}



/*********************************************************CALLBACKS*******************************************************************/
void * element_copy(void * element) {
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

void element_free(void ** element) {
    if (*element == NULL) return;
    else {
        free(*element);
        *element = NULL;
    }
}

int element_compare(void * x, void * y) {
    return ((((my_sensor_t*)x)->sid < ((my_sensor_t*)y)->sid) ? -1 : (((my_sensor_t*)x)->sid == ((my_sensor_t*)y)->sid) ? 0 : 1);
}

void element_print(void* element) {
    char buff[25];
    struct tm* timeinfo;
    timeinfo = localtime(&(((my_sensor_t*)element)->lm));
    strftime(buff, 25, "%c", timeinfo);
    printf(" (rid$=%d, sid=%d, ravg = %1.2f, ts = %s\n",  
        ((my_sensor_t*)element)->rid, ((my_sensor_t*)element)->sid, ((my_sensor_t*)element)->ravg, buff);
}






