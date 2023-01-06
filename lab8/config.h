/**
 * \author Pieter Hanssens
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define SBUFF_READER_THREADS 2
#define SBUFF_WRITER_THREADS 1

#define SBUFF_DEBUG

#include <stdint.h>
#include <time.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

#endif /* _CONFIG_H_ */
