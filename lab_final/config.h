/**
 * \author Pieter Hanssens
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

//#define DEBUG_ALL
#define DEBUG_SBUFFER
#define DEBUG_MAIN
#define ERROR_NO_EXIT

#ifdef DEBUG_ALL
#define DEBUG_DPLIST
#define DEBUG_SENSOR_DB
#define DEBUG_DATAMGR
#define DEBUG_CONNMGR
#define DEBUG_SBUFFER
#define DEBUG_MAIN
#else
#endif

#define SBUFF_READER_THREADS 1
#define SBUFF_WRITER_THREADS 1
#define SBUFF_WRITE_BUFFER 3

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 27
#endif
#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 14
#endif


//#define TIMEOUT 10
#define PORT 5678


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
