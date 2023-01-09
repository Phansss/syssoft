/**
 * \author Pieter Hanssens
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define _GNU_SOURCE
#define DEBUG_ALL

#ifdef DEBUG_ALL
#define DEBUG_SBUFF
#define DEBUG_DPLIST
#endif

#define ERR_SEMAPHORE_INIT "Error during semaphore initialization"
#define ERR_BARRIER_INIT "Error during barrier initialization" 
#define ERR_RWLOCK_INIT "Error during rwlock initialization"
#define ERR_MUTEX_INIT "Error during mutex initialization"
#define ERR_MALLOC(type) "Not enough heap memory available for " #type 

#define DMSG_MUTEX_LOCKTRY(thr,lock) #thr " tries lock: " #lock
#define DMSG_MUTEX_LOCKACQ(thr,lock) #thr " acquires lock: " #lock
#define DMSG_MUTEX_UNLOCK(thr,lock) #thr " unlocks: " #lock
#define DMSG_BARRIER_BLOCK(thr,lock) #thr " blocks on: " #lock
#define DMSG_BARRIER_UNBLOCK(thr) #thr " continues. " 


#define SBUFF_READER_THREADS 2
#define SBUFF_WRITER_THREADS 1
#define SBUFF_WRITE_BUFFER 3

#define SET_MAX_TEMP 27
#define SET_MIN_TEMP 14



#define CONNMGR_TIMEOUT 10
#define CONNMGR_PORT 5678

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "errmacros.h"
#include <malloc.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

#endif /* _CONFIG_H_ */
