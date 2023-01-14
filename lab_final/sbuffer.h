/**
 * \author Pieter Hanssens
 */
#include "config.h"


#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1

#define SBUFFER_CBW_EXIT -1

#define SBUFFER_READER 0
#define SBUFFER_WRITER 1

#define DATA_PROCESS_SUCCESS 0
#define DATA_PROCESS_ERROR 1


#ifndef SBUFFER_READER_THREADS
#error "number of reader threads not defined"
#endif
#ifndef SBUFFER_WRITER_THREADS
#error "number of writer threads not defined"
#endif

#define ERR_SBUFF_INIT "Not enough malloc memory for sbuffer"

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



#ifndef SBUFFER_SETTINGS_SET
#ifndef SBUFFER_CB_MASK
#define SBUFFER_DEFINE_CB_MASK long int cb_mask
#define SBUFFER_CB_MASK cb_mask
#endif
#define SBUFFER_RW_BUFFER_SIZE 5
#define SBUFFER_DEFINE_CB_FLAGS short int cb_flags = 0b1111;

#define SBUFFER_GET_CB_MASK(idx)                                                                           \
    do{short int SBUFFER_CB_MASK = 0;                                                                              \
        if (idx == 1) SBUFFER_CB_MASK = 0b1;   /*1*/                                                               \
        if (idx == 2) SBUFFER_CB_MASK = 0b10;  /*2*/                                                               \
        if (idx == 3) SBUFFER_CB_MASK = 0b11;  /*3*/                                                               \
        if (idx == 4) SBUFFER_CB_MASK = 0b100; /*4*/                                                               \
    }while(0)                           //add more if you need more than 4 readers
#else
#ifndef SBUFFER_CB_MASK
#define DEFINE_SBUFFER_CB_MASK long int cb_mask
#define SBUFFER_CB_MASK cb_mask
#endif
#endif

typedef struct sbuffer sbuffer_t;

typedef struct data_buffer {
    sensor_data_t* head;
    size_t size;
    int count_received;
} data_buffer_t;

typedef struct cb_args {
    data_buffer_t* data_buffer;    // pointer to data struct initalized on thread stack.
    // void* other;   // user defined struct containing other arguments to process the data
} cb_args_t;

typedef int (*cb_func)(cb_args_t*);

typedef struct sbuff_callback {
    cb_func function;
    cb_args_t* arguments;
    pthread_mutex_t* exec_lock;
} sbuff_callback_t;


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
int sbuffer_remove(sbuffer_t *buffer);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \param reader_callbacks a pointer to a list containing callback functions that need to process this sbuff node.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data);


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

/** Adds the callback function to the reader/writer thread operating on the sbuffer.
 * TO_DO: Make adding callbacks thread safe.
 * \param buffer
 * \param function function pointer to the user defined callback function
 * \param args pointer to the callback arguments on the user stack. 
 *             Note that the user is resposible to not modify these while the threads operate!
 * \param rw callback sbuffer type: 'SBUFFER_READER' or 'SBUFFER_WRITER'  
*/
int sbuffer_add_callback(sbuffer_t* buffer, cb_func function, cb_args_t* args, int rw);

/** Removes the callback function from the reader/writer thread operating on the sbuffer. Nothing happens if the function is
 *  not in the list.
 *  TO_DO: Make removing callbacks thread safe.
 * \param buffer
 * \param function function pointer to the user defined callback function
 * \param args pointer to the callback arguments on the user stack. 
 *             Note that the user is resposible to not modify these while the threads operate!
*/
int sbuffer_remove_callback(sbuffer_t* buffer, cb_func function, cb_args_t* args);


#endif  //_SBUFFER_H_
