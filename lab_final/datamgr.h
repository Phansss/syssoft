/**
 * \author Pieter Hanssens
 */

#ifndef DATAMGR_H_
#define DATAMGR_H_

#include "debugger.h"
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "lib/dplist.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#error "SET_MAX_TEMP not set"
#endif

#ifndef SET_MIN_TEMP
#error "SET_MIN_TEMP not set"
#endif


typedef struct my_sensor my_sensor_t;

extern dplist_t* sensor_room_list;

#define DATAMGR_SUCCESS 0
#define DATAMGR_FAILURE 1

/*****************************************************LAB METHODS**************************************************************/

/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors();


/** Parse the sensors from directory file room_sensor.map to the global sensor
 * \param fp_sensor_map pointer to the opened file stream.
 * \param dplist the dplist in which to store the sensors.
*/
void parse_sensor_map(FILE *fp_sensor_map, dplist_t** dplist);

/** Reads sensor data from a binary file in the buffer and updates the appropriate sensors in sensor_list.
 * \param data data struct with current sensor id, value and timestamp
*/
void process_sensor_data(sensor_data_t* data);

/**********************************************************SENSOR*********************************************************************/
/**
* Create a my_sensor_t element on heap and assign the given id and name.
* \param element new null-pointer, initialized on the caller-stack
* \param sid the sensor id of the new element.
* \param rid the room id of the new element.
* \param ravg the running
* \return void
*/
int sensor_create(my_sensor_t** sensor, sensor_id_t sid, uint16_t rid);

/** Searches for sensor with sid in the given sensor_dplist
 * \param sensor_dplist The list in which to search for the sensor
 * \param sid The ID of the sensor to look for
 * \return pointer to the sensor in dplist.
 * \return NULL when the sensor was not found.
*/
my_sensor_t* sensor_search(dplist_t* sensor_dplist, sensor_id_t sid);

/** Inserts a value in the running values buffer of the sensor and updates the running average. 
 * Also Updates the lastmodified flag with timestamp ts.
 * \param sensor the sensor to update
 * \param value the value to push to the sensor buffer
 * \param ts last modified timestamp
*/
int sensor_update(my_sensor_t* sensor, sensor_value_t value, sensor_ts_t ts);

/* void sensor_print(my_sensor_t* sensor); */

/** Pushes the given sensor value to the running values array buffer of the sensor.
 * \param sensor pointer to the sensor
 * \param value sensor value to be pushed
*/
int sensor_push_value(my_sensor_t* sensor, sensor_value_t value);

/** Updates the running average of the sensor using the running values of the sensor 
 * \param sensor pointer to the sensor to be updated.
 * \return void
*/
int sensor_update_ravg(my_sensor_t* sensor);


void * my_sensor_copy(void * element);
void my_sensor_free(void ** element) ;
int my_sensor_compare(void * x, void * y);
void my_sensor_print(void* element) ;

#endif  //DATAMGR_H_
