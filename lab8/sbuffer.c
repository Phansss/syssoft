/**
 * \author Pieter Hanssens
 */


#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "sbuffer.h"
#include <pthread.h>
#include <semaphore.h>
#include "errmacros.h"
#include <assert.h>
#include "config.h"


#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>


                                                                                

pthread_barrier_t sbuff_start_readers; // initialize in initializer.
pthread_barrier_t sbuff_main_rw_sync; // initialize in initializer.
pthread_barrier_t sbuff_reader_sync; // initialize in initializer
pthread_mutex_t sbuff_reader_removing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sbuff_reader_processing = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t sbuff_writer_inserting = PTHREAD_RWLOCK_INITIALIZER;
pthread_cond_t sbuff_writer_buffered = PTHREAD_COND_INITIALIZER;
sem_t sbuff_node_count; // initialized in writer

sbuffer_t* buffer = NULL;

int count;

pthread_t writer;
pthread_t reader1;
pthread_t reader2;
extern FILE* binary_file_out;
//pthread_t reader2;

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int cleanup) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_NO_DATA;
             // if not cleanup, lock data when reading
    pthread_mutex_lock(&sbuff_reader_removing);
    *data = buffer->head->data;
    dummy = buffer->head;
    buffer->head = buffer->head->next;
    pthread_mutex_unlock(&sbuff_reader_removing);
    free(dummy);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE; // buffer must be initialized
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;

    if (buffer->tail == NULL) // buffer empty 
    {   
        buffer->head = buffer->tail = dummy;

    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }

    return SBUFFER_SUCCESS;
}

int sbuffer_size(sbuffer_t* buffer) {
    int size = 0;
    if (buffer == NULL) return 0;

    //
    if (buffer->head == NULL) return 0;

    sbuffer_node_t* dummy = buffer->head;
    while (dummy != buffer->tail) {
        size++;
        dummy = dummy->next;
    }
    size++;
    return size;
}

pid_t sbuffer_get_tid() {
    return syscall(__NR_gettid);
}


void *writer_thread(void *arg) {
    SBUFF_DEBUGGER("Writer thread started")
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0};
    assert((dataprocessor_arg_t*)(((sbuff_thread_arg_t*)arg)->data_processor_arguments)->data == NULL); // dataprocessor arg must be initialized as null pointer
    ((dataprocessor_arg_t*)(((sbuff_thread_arg_t*)arg)->data_processor_arguments))->data = &data;       //pass stack data structure to user callback function.
    int sbuff_count = 0;
    
    //Call user callback data processor
    SBUFF_DEBUGGER("Writer starts buffering.")
    while ((((sbuff_thread_arg_t*)arg)->data_processor)(((sbuff_thread_arg_t*)arg)->data_processor_arguments)!= DATA_PROCESS_ERROR) {
        sbuffer_insert(((sbuff_thread_arg_t*)arg)->sbuffer, &data);
        sbuff_count++;
        if (sbuff_count == SBUFF_READER_THREADS + 1) break; // buffer until sbuffer contians SBUFF_READER_THREADS + 1 data nodes
    }
    SBUFF_DEBUGGER("Writer buffered %d sbuffer nodes.", sbuffer_size(((sbuff_thread_arg_t*)arg)->sbuffer))

    // TO DO:   implement mechanism so that multiple writers can lock the rwlock!
    SBUFF_DEBUGGER("Writer lock rwlock 'sbuff_writer_inserting'")
    pthread_rwlock_wrlock(&sbuff_writer_inserting); //SBUFFERED LOCK ON
    SBUFF_DEBUGGER("Writer blocking on barrier 'sbuff_start_readers'")
    pthread_barrier_wait(&sbuff_start_readers);
    SBUFF_DEBUGGER("Writer continue writing")
    while ((((sbuff_thread_arg_t*)arg)->data_processor)(((sbuff_thread_arg_t*)arg)->data_processor_arguments)!= DATA_PROCESS_ERROR) {
        sbuffer_insert(((sbuff_thread_arg_t*)arg)->sbuffer, &data);
        sbuff_count++;
        sem_post(&sbuff_node_count);
        sched_yield();         
    }
    SBUFF_DEBUGGER("Writer finished. Wrote %d sbuffer nodes", sbuff_count)
      
    // TO DO:   implement mechanism so that multiple writers can unlock the rwlock! (see above)
    SBUFF_DEBUGGER("Writer unlocking for buffered cleanup")
    pthread_rwlock_unlock(&sbuff_writer_inserting);
    
    SBUFF_DEBUGGER("Writer exiting")
    pthread_exit(0);
}

void *reader_thread(void *arg) {
    SBUFF_DEBUGGER("Reader thread started")
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0}; // data is thread safe variable
    assert((dataprocessor_arg_t*)(((sbuff_thread_arg_t*)arg)->data_processor_arguments)->data == NULL); // dataprocessor arg must be initialized as null pointer
    ((dataprocessor_arg_t*)(((sbuff_thread_arg_t*)arg)->data_processor_arguments))->data = &data;       // pass stack data structure to user callback function.
    SBUFF_DEBUGGER("Reader blocking on barrier 'sbuff_start_readers'")
    pthread_barrier_wait(&sbuff_start_readers);                                                         // Wait until all writers have buffered.

    int result = 0;
    SBUFF_DEBUGGER("Reader starts reading ")
    while(1) {
        if (pthread_rwlock_tryrdlock(&sbuff_writer_inserting) == EBUSY) {               // writer still busy; lock not acquired.
            if (sem_trywait(&sbuff_node_count) == 0) {                                  // block until resources are available in the sbuffer
                sbuffer_remove(((sbuff_thread_arg_t*)arg)->sbuffer, &data, 0);          // Remove node at the start of the sbuffer -> no interference with inserted nodes at the end
                //printf("(%d): sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", x, data.id, data.value, (long int) data.ts);
                //printf("READER: blocking on sbuff_reader_processing\n");
                pthread_mutex_lock(&sbuff_reader_processing);
                (((sbuff_thread_arg_t*)arg)->data_processor)(((sbuff_thread_arg_t*)arg)->data_processor_arguments);
                count++;
                pthread_mutex_unlock(&sbuff_reader_processing);
                continue;
            } else if (pthread_rwlock_tryrdlock(&sbuff_writer_inserting) == EBUSY) {
                continue;
            } else {
                SBUFF_DEBUGGER("Reader unlock rwlock 'sbuff_writer_inserting'")
                pthread_rwlock_unlock(&sbuff_writer_inserting); // current thread jumps to else                                                  
            }
        } else {
            SBUFF_DEBUGGER("Reader unlock rwlock 'sbuff_writer_inserting'")
            pthread_rwlock_unlock(&sbuff_writer_inserting);  // LOCK ACQUIRED! -> unlock for other reader threads. 
            SBUFF_DEBUGGER("Reader blocking on 'sbuff_reader_sync'")
            result = pthread_barrier_wait(&sbuff_reader_sync);                 // wait for other reader!
            if(result == 0) {    //  close every reader thread except 1
                SBUFF_DEBUGGER("Reader exiting'")                             
                pthread_exit(0);
            }
            SBUFF_DEBUGGER("Reader cleaning up %d sbuffer nodes'", sbuffer_size(((sbuff_thread_arg_t*)arg)->sbuffer))
            while(sbuffer_size(((sbuff_thread_arg_t*)arg)->sbuffer) > 0) {
                sbuffer_remove(((sbuff_thread_arg_t*)arg)->sbuffer, &data, 1);
                (((sbuff_thread_arg_t*)arg)->data_processor)(((sbuff_thread_arg_t*)arg)->data_processor_arguments);               
                count++;
            }

            SBUFF_DEBUGGER("Reader exiting'") 
            pthread_exit(0);
        }
    }

    

}




