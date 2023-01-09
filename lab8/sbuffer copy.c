/**
 * \author Pieter Hanssens
 */


#define _GNU_SOURCE

#include "sbuffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include "errmacros.h"
#include <assert.h>
#include "config.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "lib/dplist.h"


#define _sbuffer_ (((sbuff_thread_arg_t*)arg)->sbuffer)
#define _callback_arg_ (((sbuff_thread_arg_t*)arg)->data_processor_arguments)
#define _callback_func_ (((sbuff_thread_arg_t*)arg)->data_processor)
#define _node_callbacks_ (((sbuff_thread_arg_t*)arg)->sbuff_thread_callbacks)

pid_t tid;
#ifdef SBUFF_DEBUG
#define SBUFF_ERROR(...) SBUFF_ERROR_EXIT(__VA_ARGS__, " ");                                  
#define SBUFF_ERROR_EXIT(cond,fmt,...) 									                                                    \
        do {if (cond) {                                                                                                                \
                tid = syscall(__NR_gettid);                                                                                     \
                fprintf(stderr, "DEBUG_SBUFF [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,tid, __VA_ARGS__);    \
                fprintf(stderr, "  Error because: "#cond"\n");                                                                  \
                fflush(stderr);                                                                                                 \
                exit(1);                                                                                                        \
            }                                                                                                                   \
            }while(0)

#define SBUFF_DEBUGGER_PRINTF(...) SBUFF_DEBUG_PRINTF(__VA_ARGS__, " ");                                  
#define SBUFF_DEBUG_PRINTF(fmt,...) 									                                                    \
        do {                                                                                                                \
            tid = syscall(__NR_gettid);                                                                                     \
            fprintf(stderr, "DEBUG_SBUFF [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,tid, __VA_ARGS__);    \
            fflush(stderr);                                                                                                 \
            }while(0)
#else
#define SBUFF_ERROR(...) SBUFF_ERROR_NO_EXIT(__VA_ARGS__, " ");                                  
#define SBUFF_ERROR_NO_EXIT(cond,fmt,...) 									                                                    \
        do {if (cond) {                                                                                                                \
                tid = syscall(__NR_gettid);                                                                                     \
                fprintf(stderr, "  Error because: "#cond"\n");                                                                  \
                fflush(stderr);                                                                                                 \
            }                                                                                                                   \
            }while(0)
#define SBUFF_DEBUGGER_PRINTF(...) SBUFF_DEBUG_PRINTF(__VA_ARGS__, " ");
#define SBUFF_DEBUG_PRINTF(fmt,...) (void)0
#endif
                //fprintf(stderr, "ERR_SBUFF  [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,tid, __VA_ARGS__);    
int count;

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    sem_t* semaphore_r_processedcount; // initialized at strt of node
    pthread_barrier_t* barrier_r_nodesync;
    r_callback_func_t* r_callbacks;
    int index_w_mutex_buffer; // index of the mutex lock array.
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
    pthread_mutex_t* mutex_r_remove;
    pthread_mutex_t* mutex_r_processing;
    pthread_mutex_t* mutex_r_buffer;
    pthread_mutex_t* mutex_w_calc_mbuffer_index;
    pthread_rwlock_t* rwlock_rw_writing;
    pthread_barrier_t* barrier_r_readerssync;
    pthread_barrier_t* barrier_rw_startreaders;
    pthread_barrier_t* barrier_r_nodesync;
    Dataprocessingfunc* sbuff_r_callbacks;
    sem_t* semaphore_rw_nodecount; // initialized in writer
    sem_t* semaphore_r_nodecount_cleanup; // initialized at start of cleanup
    int no_readers;
    int no_writers;
    int index_w_mutex_buffer;
};

int sbuffer_init(sbuffer_t **buffer, int no_readers, int no_writers) {
    *buffer = (sbuffer_t*) malloc(sizeof(sbuffer_t));
    SBUFF_ERROR((*buffer) == NULL, "Not enough malloc memory for sbuffer");
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    (*buffer)->mutex_r_remove = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    (*buffer)->no_readers = no_readers;
    (*buffer)->no_writers = no_writers;
    SBUFF_ERROR((*buffer)->mutex_r_remove == NULL, "Not enough malloc memory for mutex");
    SBUFF_ERROR((pthread_mutex_init((*buffer)->mutex_r_remove, NULL) == -1), "Error during mutex initialization");
    (*buffer)->mutex_r_processing = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    SBUFF_ERROR(((*buffer)->mutex_r_processing == NULL), "Not enough malloc memory for mutex");
    SBUFF_ERROR((pthread_mutex_init((*buffer)->mutex_r_processing, NULL) == -1), "Error during mutex initialization");
    (*buffer)->rwlock_rw_writing = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t)); 
    SBUFF_ERROR((*buffer)->rwlock_rw_writing == NULL, "Not enough malloc memory for rwlock");
    SBUFF_ERROR((pthread_rwlock_init((*buffer)->rwlock_rw_writing, NULL) == -1), "Error during rwlock initialization");
    (*buffer)->semaphore_rw_nodecount = (sem_t*) malloc(sizeof(sem_t));
    SBUFF_ERROR((*buffer)->semaphore_rw_nodecount == NULL, "Not enough malloc memory for semaphore");
    SBUFF_ERROR((sem_init((*buffer)->semaphore_rw_nodecount,  0, 0) == -1), "Error during semaphore initialization");
    (*buffer)->semaphore_r_nodecount_cleanup = (sem_t*) malloc(sizeof(sem_t));
    SBUFF_ERROR((*buffer)->semaphore_r_nodecount_cleanup == NULL, "Not enough malloc memory for semaphore"); // initialization at cleanup
    (*buffer)->barrier_r_readerssync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    SBUFF_ERROR((*buffer)->barrier_r_readerssync == NULL, "Not enough malloc memory for barrier");
    SBUFF_ERROR((pthread_barrier_init((*buffer)->barrier_r_readerssync, NULL, no_readers) == -1), "Error during barrier initialization");
    (*buffer)->barrier_r_nodesync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    SBUFF_ERROR((*buffer)->barrier_r_nodesync == NULL, "Not enough malloc memory for barrier");
    SBUFF_ERROR((pthread_barrier_init((*buffer)->barrier_r_nodesync, NULL, no_readers) == -1), "Error during barrier initialization");
    (*buffer)->barrier_rw_startreaders = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    SBUFF_ERROR((*buffer)->barrier_rw_startreaders == NULL, "Not enough malloc memory for barrier");
    SBUFF_ERROR((pthread_barrier_init((*buffer)->barrier_rw_startreaders, NULL, no_readers + no_writers) == -1), "Error during barrier initialization");

    (*buffer)->mutex_r_buffer = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)*no_readers);
    SBUFF_ERROR((*buffer)->mutex_r_buffer == NULL, "Not enough malloc memory for rwlock");
    for (int i; i<no_readers; i++) {
        SBUFF_ERROR((pthread_mutex_init((&((*buffer)->mutex_r_buffer)[i]) , NULL) == -1), "Error during mutex initialization");
    }
    (*buffer)->mutex_w_calc_mbuffer_index = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)*no_readers);
    SBUFF_ERROR((*buffer)->mutex_w_calc_mbuffer_index == NULL, "Not enough malloc memory for rwlock");
    SBUFF_ERROR((pthread_mutex_init((*buffer)->mutex_w_calc_mbuffer_index , NULL) == -1), "Error during mutex initialization");


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
    sem_destroy((*buffer)->semaphore_rw_nodecount);
    pthread_barrier_destroy((*buffer)->barrier_r_readerssync);
    pthread_barrier_destroy((*buffer)->barrier_rw_startreaders);
    pthread_barrier_destroy((*buffer)->barrier_r_nodesync);
    pthread_mutex_destroy((*buffer)->mutex_r_processing);
    pthread_mutex_destroy((*buffer)->mutex_r_remove);
    pthread_rwlock_destroy((*buffer)->rwlock_rw_writing);
    free((*buffer)->semaphore_rw_nodecount);
    free((*buffer)->barrier_r_readerssync);
    free((*buffer)->barrier_rw_startreaders);
    free((*buffer)->barrier_r_nodesync);
    free((*buffer)->mutex_r_processing);
    free((*buffer)->mutex_r_remove);
    free((*buffer)->rwlock_rw_writing);
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}


int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int cleanup) {
    sbuffer_node_t *dummy;
    SBUFF_DEBUGGER_PRINTF("locking accessing buffer");
    if (buffer == NULL) return SBUFFER_FAILURE;
    SBUFF_DEBUGGER_PRINTF("locking accessing buffer->head");
    if (buffer->head == NULL) return SBUFFER_NO_DATA;
             // if not cleanup, lock data when reading
    
    //pthread_barrier_wait(buffer->barrier_r_nodesync);
     //fetch data to reader stack
    //SBUFF_DEBUG_PRINTF("Reader blocking on 'barrier_r_nodesync", " ");
    
    //if (sem_trywait(dummy->semaphore_r_processedcount) != 0) { // every reader has fetched the data
        //SBUFF_DEBUG_PRINTF("Reader removing node", " ");

    
    SBUFF_DEBUGGER_PRINTF("locking mutex_r_remove");

    pthread_mutex_lock(buffer->mutex_r_remove);
    dummy = buffer->head;
    *data = buffer->head->data;
    SBUFF_DEBUGGER_PRINTF("removing sbuff with mutex index = %d", dummy->index_w_mutex_buffer);
    SBUFF_DEBUGGER_PRINTF("buffer->head: %p", buffer->head->next);
    buffer->head = buffer->head->next;
    pthread_mutex_unlock(buffer->mutex_r_remove);
    SBUFF_DEBUGGER_PRINTF("unlocking mutex_r_remove");
    
    SBUFF_DEBUGGER_PRINTF("destroying mutex_r_remove")
    sem_destroy(dummy->semaphore_r_processedcount);
    pthread_barrier_destroy(dummy->barrier_r_nodesync);
    free(dummy->semaphore_r_processedcount);
    free(dummy->barrier_r_nodesync);
    free(dummy);
    //} else {
    //    SBUFF_DEBUG_PRINTF("Reader continuing", " ");
    //    pthread_barrier_wait(dummy->barrier_r_nodesync);
    //}
    
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data, dplist_t* reader_callbacks) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE; // buffer must be initialized
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL; 

    //dummy->semaphore_r_processedcount = (sem_t*) malloc(sizeof(sem_t));
    //SBUFF_ERROR(dummy->semaphore_r_processedcount == NULL, "Not enough malloc memory for semaphore in sbuffer_node");
    //SBUFF_ERROR((sem_init(dummy->semaphore_r_processedcount,  0, ((buffer)->no_readers)) == -1), "Error during semaphore initialization");

    //Inserting function callbacks
    dummy->r_callbacks = (r_callback_func_t*) malloc(sizeof(r_callback_func_t)*dpl_size(reader_callbacks));
    SBUFF_ERROR(dummy->r_callbacks == NULL, "Not enough malloc memory for reader_callbacks in sbuffer_node");
    r_callback_func_t* temp_callback;
    for (int i= 0; i < dpl_size(reader_callbacks); i++) {
        temp_callback = (r_callback_func_t*) dpl_get_element_at_index(reader_callbacks, i);
        (dummy->r_callbacks)[i].callback = temp_callback->callback;
        (dummy->r_callbacks)[i].callback_arg = temp_callback->callback_arg;
        (dummy->r_callbacks)[i].execute_count = temp_callback->execute_count;
    }
    

    //dummy->barrier_r_nodesync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    //SBUFF_ERROR(dummy->barrier_r_nodesync == NULL, "Not enough malloc memory for barrier");
    //SBUFF_ERROR((pthread_barrier_init(dummy->barrier_r_nodesync, NULL, (buffer)->no_readers) == -1), "Error during barrier initialization");
    
    pthread_mutex_lock(buffer->mutex_w_calc_mbuffer_index);
    
    buffer->index_w_mutex_buffer++;
    if (buffer->index_w_mutex_buffer == buffer->no_readers) buffer->index_w_mutex_buffer = 0;
    dummy->index_w_mutex_buffer = buffer->index_w_mutex_buffer;
    pthread_mutex_unlock(buffer->mutex_w_calc_mbuffer_index);
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

void *reader_thread(void *arg) {
    SBUFF_DEBUGGER_PRINTF("Reader thread started")
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0}; // data is thread safe variable
    assert((dataprocessor_arg_t*)(_callback_arg_)->data == NULL); // dataprocessor arg must be initialized as null pointer
    ((dataprocessor_arg_t*)(_callback_arg_))->data = &data;       // pass stack data structure to user callback function.
    SBUFF_DEBUGGER_PRINTF("Reader blocking on barrier 'barrier_rw_startreaders'")
    pthread_barrier_wait(_sbuffer_->barrier_rw_startreaders);                                                         // Wait until all writers have buffered.
    
    int result = 0;
    SBUFF_DEBUGGER_PRINTF("Reader starts reading ")
    while(1) {
        if (pthread_rwlock_tryrdlock(_sbuffer_->rwlock_rw_writing) == EBUSY) {               // writer still busy; lock not acquired.
            if (sem_trywait(_sbuffer_->semaphore_rw_nodecount) == 0) {                   
                sbuffer_remove(_sbuffer_, &data, 0);          // Remove node at the start of the sbuffer -> no interference with inserted nodes at the end
                //SBUFF_DEBUGGER_PRINTF("Reader lock 'r mutex_processing'")
                pthread_mutex_lock(_sbuffer_->mutex_r_processing);
                (_callback_func_)(_callback_arg_);
                count++;
                pthread_mutex_unlock(_sbuffer_->mutex_r_processing);
                continue;

            } else if (pthread_rwlock_tryrdlock(_sbuffer_->rwlock_rw_writing) == EBUSY) {
                continue;
            } else {
                SBUFF_DEBUGGER_PRINTF("Reader unlock rwlock 'rwlock_rw_writing'")
                pthread_rwlock_unlock(_sbuffer_->rwlock_rw_writing); // current thread jumps to else                                                  
            }
        } else {
            SBUFF_DEBUGGER_PRINTF("Reader unlock rwlock 'rwlock_rw_writing'")
            pthread_rwlock_unlock(_sbuffer_->rwlock_rw_writing);                             // LOCK ACQUIRED! -> unlock for other reader threads. 
            SBUFF_DEBUGGER_PRINTF("Reader blocking on 'barrier_r_readerssync'")
            result = pthread_barrier_wait(_sbuffer_->barrier_r_readerssync);                 // wait for other readers!
            
            if(result == 0) {                                                                //  close every reader thread except 1
                SBUFF_DEBUGGER_PRINTF("Reader going to sleep'")                             
                pthread_barrier_wait(_sbuffer_->barrier_r_readerssync);                      //hold every reader but one
            } else {
                SBUFF_DEBUGGER_PRINTF("Reader still awake, determined %d remaining sbuffer nodes'", sbuffer_size(_sbuffer_))
                SBUFF_ERROR((sem_init(_sbuffer_->semaphore_r_nodecount_cleanup,  0, sbuffer_size(_sbuffer_)) == -1), "Error during semaphore initialization");
                pthread_barrier_wait(_sbuffer_->barrier_r_readerssync); //wake up other readers
            }            
            while (sem_trywait(_sbuffer_->semaphore_r_nodecount_cleanup) == 0) {                   
                sbuffer_remove(_sbuffer_, &data, 0);          // Remove node at the start of the sbuffer -> no interference with inserted nodes at the end
                pthread_mutex_lock(_sbuffer_->mutex_r_processing);
                (_callback_func_)(_callback_arg_);
                count++;
                pthread_mutex_unlock(_sbuffer_->mutex_r_processing);
                
                continue;
            }
            SBUFF_DEBUGGER_PRINTF("Reader exiting'")
            SBUFF_DEBUGGER_PRINTF("Read %d data points from sbuffer", count); 
            pthread_exit(0);
        }
    }

    

}

void *writer_thread(void *arg) {
    
    SBUFF_DEBUGGER_PRINTF("Writer thread started")
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0};
    assert((dataprocessor_arg_t*)(_callback_arg_)->data == NULL); // dataprocessor arg must be initialized as null pointer by the user
    ((dataprocessor_arg_t*)(_callback_arg_))->data = &data;       //pass stack data structure to user callback function.
    int sbuff_count = 0;
    
    //Call user callback data processor
    SBUFF_DEBUGGER_PRINTF("Writer starts buffering.")
    while ((_callback_func_)(_callback_arg_)!= DATA_PROCESS_ERROR) {
        sbuffer_insert(_sbuffer_, &data, _node_callbacks_);
        sbuff_count++;
        if (sbuff_count == SBUFF_READER_THREADS + 1) break; // buffer until sbuffer contians SBUFF_READER_THREADS + 1 data nodes
    }
    SBUFF_DEBUGGER_PRINTF("Writer buffered %d sbuffer nodes.", sbuffer_size(_sbuffer_))

    // TO DO:   implement mechanism so that multiple writers can lock the rwlock!
    SBUFF_DEBUGGER_PRINTF("Writer lock rwlock 'rwlock_rw_writing'")
    pthread_rwlock_wrlock(_sbuffer_->rwlock_rw_writing); //SBUFFERED LOCK ON

    SBUFF_DEBUGGER_PRINTF("Writer blocking on barrier 'barrier_rw_startreaders'")
    pthread_barrier_wait(_sbuffer_->barrier_rw_startreaders);

    SBUFF_DEBUGGER_PRINTF("Writer continue writing")
    while ((_callback_func_)(_callback_arg_)!= DATA_PROCESS_ERROR) {
        sbuffer_insert(_sbuffer_, &data, _node_callbacks_);
        sbuff_count++;
        sem_post(_sbuffer_->semaphore_rw_nodecount);
        sched_yield();         
    }
    SBUFF_DEBUGGER_PRINTF("Writer finished. Wrote %d sbuffer nodes", sbuff_count)
      
    // TO DO:   implement mechanism so that multiple writers can unlock the rwlock! (see above)
    SBUFF_DEBUGGER_PRINTF("Writer unlocking for buffered cleanup")
    pthread_rwlock_unlock(_sbuffer_->rwlock_rw_writing);
    
    SBUFF_DEBUGGER_PRINTF("Writer exiting")
    pthread_exit(0);
    
}


// not thread safe!
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

