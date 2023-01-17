/**
 * \author Pieter Hanssens
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

//___________________________________________________ GENERAL APPLICATION SETTINGS ____________________________________________
#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 27
#endif
#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 14
#endif

#ifndef TIMEOUT
#define TIMEOUT 10
#endif

#ifndef PORT
#define PORT 5678
#endif
                              
#define ERROR_EXIT break //define actions to be taken after error detection. 'break' for no action  /              

#ifdef DEBUG_ALL
#define DEBUG_DPLIST
#define DEBUG_SENSOR_DB
#define DEBUG_DATAMGR
#define DEBUG_CONNMGR
#define DEBUG_SBUFFER
#define DEBUG_MAIN
#else
//#define DEBUG_DPLIST
//#define DEBUG_SENSOR_DB
//#define DEBUG_DATAMGR
//#define DEBUG_CONNMGR
#define DEBUG_SBUFFER
//#define DEBUG_MAIN
#endif
//---------------------------------------------------------------------------------------------------------------------------------------


//__________________________________________________________ SBUFFER SETTINGS ___________________________________________________________

//__THREAD SETTINGS
#define SBUFFER_READER_THREADS 2 // Number of reader threads
#define SBUFFER_WRITER_THREADS 1 // Number of sbuffer writer threads
#define SBUFFER_RW_BUFFER_SIZE 5 // Number of data elements processed in one (sbuff_writer) callback function.
                                                             //add more if you need more than 4 reader callbacks
#define SBUFFER_SETTINGS_SET    //if not defined, use default settings in sbuffer.h
//________________________________________________________________________________________________________________________________________




//#include <stdint.h>
//#include <malloc.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

#endif /* _CONFIG_H_ */
