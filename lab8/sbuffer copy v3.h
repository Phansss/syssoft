/**
 * \author Pieter Hanssens
 */
#include "config.h"
#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1

#define DATA_PROCESS_SUCCESS 0
#define DATA_PROCESS_ERROR 1

#ifndef SBUFF_READER_THREADS
#error "number of reader threads not defined"
#endif
#ifndef SBUFF_WRITER_THREADS
#error "number of writer threads not defined"
#endif


#define PTH_create( a, b, c, d ) \
    (pthread_create( (a), (b), (c), (d) ) != 0 ? abort() : (void)0 )

#define PTH_rwlock_rdlock( a ) \
    (pthread_rwlock_rdlock( (a) ) != 0 ? abort() : (void)0 )

#define PTH_rwlock_wrlock( a ) \
    (pthread_rwlock_wrlock( (a) ) != 0 ? abort() : (void)0 )

#define PTH_rwlock_unlock( a ) \
    (pthread_rwlock_unlock( (a) ) != 0 ? abort() : (void)0 )

#define PTH_join( a, b ) \
    (pthread_join( (a), (b) ) != 0 ? abort() : (void)0 )

#define NO_OF_READERS 2

typedef struct sbuffer sbuffer_t;

/**
 * attributes for the threads to access shared resources
 */





typedef struct dataprocessor_arg {
    sensor_data_t* data;    // pointer to data struct initalized on thread stack.
    // void* other;   // user defined struct containing other arguments to process the data
} dataprocessor_arg_t;

/** typefunc to define functions that receive/send data in and out of the sbuffer. 
 *  the sensor data is initialized on the writer stack and can be retrieved/modified 
 * using the pointer. 
 * The void argument is used for data processing arguments
*/
typedef int (*Dataprocessingfunc)(dataprocessor_arg_t*);

typedef struct sbuff_thread_arg {
    sbuffer_t* sbuffer;
    Dataprocessingfunc data_processor;            // pointer to data struct initalized on thread stack.
    dataprocessor_arg_t* data_processor_arguments; // void* other;   // user defined struct containing other arguments to process the data
} sbuff_thread_arg_t;






/**
 * Allocates and initializes a new shared buffer
 * \param buffer a double pointer to the buffer that needs to be initialized
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_init(sbuffer_t **buffer, int no_readers, int no_writers);

/**
 * All allocated resources are freed and cleaned up
 * \param buffer a double pointer to the buffer that needs to be freed
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_free(sbuffer_t **buffer);

/**
 * Removes the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 * If 'buffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int x);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);


int sbuffer_size(sbuffer_t* buffer); 



/** Start a new sbuffer writer that reads data from the binary file and inserts it in the buffer.
 * \param binary_file the sensor data file to read from
 * \param buffer the buffer in which to insert
*/
void* writer_thread(void* arg);

/** Start a new sbuffer reader that writes data from the sbuffer and inserts it in the binary file.
 * \param binary_file the sensor data file to write to
 * \param buffer the buffer from which to read
*/
void *reader_thread(void* arg);


#endif  //_SBUFFER_H_
