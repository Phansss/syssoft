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
#include "main_debug.h"

int test = 0;

#define _sbuffer_ (((sbuff_thread_arg_t*)arg)->sbuffer)
#define _callback_arg_ (((sbuff_thread_arg_t*)arg)->data_processor_arguments)
#define _callback_func_ (((sbuff_thread_arg_t*)arg)->data_processor)
#define _node_callbacks_ (((sbuff_thread_arg_t*)arg)->sbuff_thread_callbacks)

#ifdef DEBUG_SBUFF                                                  
#define ERROR_IF(...) _ERROR_NO_ARG(__VA_ARGS__, " ");                                  
#define _ERROR_NO_ARG(cond,fmt,...) 									                                                             \
        do {if (cond) {                                                                                                              \
                pid_t* tid = (pid_t*) malloc(sizeof(pid_t));                                                                         \
                if (tid == NULL) {                                                                                                   \
                    fprintf(stderr, "ERROR_SBUFF [%s:%d] in %s: "fmt"%s\n",  __FILE__,__LINE__,__func__, __VA_ARGS__);               \
                    fprintf(stderr,"    "#cond"\n");                                                                                 \
                    fprintf(stderr,"   NOTE: Could not determine thread id due to malloc error");                                    \
                    fflush(stderr);                                                                                                  \
                    exit(1);                                                                                                         \
                }                                                                                                                    \
                *tid = syscall(__NR_gettid);                                                                                         \
                fprintf(stderr, "ERROR_SBUFF [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,*tid, __VA_ARGS__);        \
                fprintf(stderr,"    "#cond"\n");                                                                                     \
                fflush(stderr);                                                                                                      \
                free(tid);                                                                                                           \
                exit(1);                                                                                                             \
            }                                                                                                                        \
            }while(0)
#define DEBUG_PRINTF(...) _DEBUG_NO_ARG(__VA_ARGS__, " ");                                  
#define _DEBUG_NO_ARG(fmt,...) 									                                                                     \
        do {pid_t* tid = (pid_t*) malloc(sizeof(pid_t));                                                                             \
                if (tid == NULL) {                                                                                                   \
                    fprintf(stderr, "DEBUG_SBUFF [%s:%d] in %s: "fmt"%s\n",  __FILE__,__LINE__,__func__, __VA_ARGS__);    \
                    fprintf(stderr,"   NOTE: Could not determine thread id due to malloc error");                                    \
                    fflush(stderr);                                                                                                  \
                    break;                                                                                                           \
                }                                                                                                                    \
                *tid = syscall(__NR_gettid);                                                                                         \
                fprintf(stderr, "DEBUG_SBUFF [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,*tid, __VA_ARGS__);        \
                fflush(stderr);                                                                                                      \
                free(tid);                                                                                                           \
            }while(0)            
#else
#define ERROR_IF(...) _ERROR_NO_ARG(__VA_ARGS__, " ");                                  
#define _ERROR_NO_ARG(cond,fmt,...) 									                                                             \
        do {if (cond) {
                pid_t* tid = (pid_t*) malloc(sizeof(pid_t));                                                                         \
                if (tid == NULL) {                                                                                                   \
                    fprintf(stderr, "ERROR_SBUFF [%s:%d] in %s",  __FILE__,__LINE__,__func__, __VA_ARGS__);                         \
                    fprintf(stderr,"    "#cond"\n");                                                                                 \
                    fprintf(stderr,"   NOTE: Could not determine thread id due to malloc error");                                    \
                    exit(1);                                                                                                         \
                }                                                                                                                    \
                *tid = syscall(__NR_gettid);                                                                                         \
                fprintf(stderr, "ERROR_SBUFF [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,*tid, __VA_ARGS__);       \
                fprintf(stderr,"    "#cond"\n");                                                                                     \
                fflush(stderr);                                                                                                      \
                free(tid);                                                                                                           \
            }                                                                                                                        \
            }while(0)
#define DEBUG_PRINTF(...) _DEBUG_NO_ARG(__VA_ARGS__, " ");
#define _DEBUG_NO_ARG(fmt,...) (void)0
#endif


int count;
void * element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);
void element_print(void * element);

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    int rb_index; // index of the mutex lock array.
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
    pthread_mutex_t* mutex_r_remove;
    pthread_mutex_t* mutex_r_processing;
    pthread_mutex_t** nodelock_array;
    pthread_mutex_t* mutex_w_calc_rb_index;
    dplist_t* cbs_r;
    dplist_t* cbs_w;

    pthread_rwlock_t* rwlock_rw_writing;
    pthread_barrier_t* barrier_r_readerssync;
    pthread_barrier_t* barrier_rw_startreaders;
    pthread_barrier_t* barrier_r_nodesync;
    
    sem_t* semaphore_rw_buffercount; // initialized in writer
    sem_t* semaphore_r_nodecount_cleanup; // initialized at start of cleanup
    //sem_t* semaphore_w_buffer; // initialized at start of cleanup
    int no_readers;
    int no_writers;
    int write_la_index;
};

int sbuffer_init(sbuffer_t **buffer, int no_readers, int no_writers) {
    
    *buffer = (sbuffer_t*) malloc(sizeof(sbuffer_t));
    ERROR_IF((*buffer) == NULL, ERR_SBUFF_INIT);
    (*buffer)->write_la_index = 0;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    (*buffer)->no_readers = no_readers;
    (*buffer)->no_writers = no_writers;
    ((*buffer)->cbs_r) = dpl_create(element_copy, element_free, element_compare, element_print);
    ((*buffer)->cbs_w) = dpl_create(element_copy, element_free, element_compare, element_print);  
    
    (*buffer)->rwlock_rw_writing = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t)); 
    ERROR_IF((*buffer)->rwlock_rw_writing == NULL, ERR_MALLOC("rwlock"));
    ERROR_IF((pthread_rwlock_init((*buffer)->rwlock_rw_writing, NULL) == -1), ERR_RWLOCK_INIT);
    
    (*buffer)->semaphore_rw_buffercount = (sem_t*) malloc(sizeof(sem_t));
    ERROR_IF((*buffer)->semaphore_rw_buffercount == NULL, ERR_MALLOC("semaphore"));
    ERROR_IF((sem_init((*buffer)->semaphore_rw_buffercount,  0, 0) == -1), ERR_SEMAPHORE_INIT);
    
    (*buffer)->semaphore_r_nodecount_cleanup = (sem_t*) malloc(sizeof(sem_t));
    ERROR_IF((*buffer)->semaphore_r_nodecount_cleanup == NULL, ERR_MALLOC("semaphore")); // initialization at cleanup
    


    (*buffer)->barrier_r_readerssync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_r_readerssync == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_r_readerssync, NULL, no_readers) == -1), ERR_BARRIER_INIT);
    
    
    (*buffer)->barrier_r_nodesync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_r_nodesync == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_r_nodesync, NULL, no_readers) == -1), ERR_BARRIER_INIT);

    (*buffer)->barrier_rw_startreaders = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_rw_startreaders == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_rw_startreaders, NULL, no_readers + no_writers) == -1), ERR_BARRIER_INIT);
    

    (*buffer)->barrier_rw_startreaders = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_rw_startreaders == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_rw_startreaders, NULL, no_readers + no_writers) == -1), ERR_BARRIER_INIT);
    





    (*buffer)->mutex_r_remove = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    ERROR_IF((*buffer)->mutex_r_remove == NULL, ERR_MALLOC("mutex"));
    ERROR_IF((pthread_mutex_init((*buffer)->mutex_r_remove, NULL) == -1), ERR_MUTEX_INIT);
    (*buffer)->mutex_r_processing = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    ERROR_IF(((*buffer)->mutex_r_processing == NULL), ERR_MALLOC("mutex"));
    ERROR_IF((pthread_mutex_init((*buffer)->mutex_r_processing, NULL) == -1), ERR_MUTEX_INIT);
    (*buffer)->mutex_w_calc_rb_index = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)*no_readers);
    ERROR_IF((*buffer)->mutex_w_calc_rb_index == NULL, ERR_MALLOC("mutex"));
    ERROR_IF((pthread_mutex_init((*buffer)->mutex_w_calc_rb_index , NULL) == -1), ERR_MUTEX_INIT);
    
    (*buffer)->nodelock_array = (pthread_mutex_t**) malloc(sizeof(pthread_mutex_t*)*no_readers);
    ERROR_IF((*buffer)->nodelock_array == NULL, ERR_MALLOC("mutex"));
    for (int i=0; i<no_readers; i++) {
        ((*buffer)->nodelock_array)[i] = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        ERROR_IF(((*buffer)->nodelock_array)[i] == NULL, ERR_MALLOC("mutex"));
        ERROR_IF((pthread_mutex_init((((*buffer)->nodelock_array)[i]) , NULL) == -1), ERR_MUTEX_INIT);
    }
    DEBUG_PRINTF("Initialized 'barrier_rw_startreaders' at address: %p", &(((sbuffer_t*)buffer)->barrier_rw_startreaders));
    
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
    sem_destroy((*buffer)->semaphore_rw_buffercount);
    sem_destroy((*buffer)->semaphore_r_nodecount_cleanup);
    pthread_barrier_destroy((*buffer)->barrier_r_readerssync);
    pthread_barrier_destroy((*buffer)->barrier_r_nodesync);
    pthread_barrier_destroy((*buffer)->barrier_rw_startreaders);
    pthread_mutex_destroy((*buffer)->mutex_r_remove);
    pthread_mutex_destroy((*buffer)->mutex_r_processing);
    pthread_mutex_destroy((*buffer)->mutex_w_calc_rb_index);
    for (int i=0; i<(*buffer)->no_readers; i++) {
        pthread_mutex_destroy(((*buffer)->nodelock_array)[i]);
        free(((*buffer)->nodelock_array)[i]);
    }
    
    pthread_rwlock_destroy((*buffer)->rwlock_rw_writing);
    dpl_free(&((*buffer)->cbs_r), true);
    dpl_free(&((*buffer)->cbs_w), true);
    
    free((*buffer)->semaphore_rw_buffercount);
    free((*buffer)->semaphore_r_nodecount_cleanup);
    free((*buffer)->barrier_r_readerssync);
    free((*buffer)->barrier_r_nodesync);
    free((*buffer)->barrier_rw_startreaders);
    free((*buffer)->mutex_r_remove);
    free((*buffer)->mutex_r_processing);
    free((*buffer)->mutex_w_calc_rb_index);
    free((*buffer)->nodelock_array);
    free((*buffer)->rwlock_rw_writing);
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

/** Returns the current size of the sbuffer
 * //TO_DO: make thread safe!
 * 
*/
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

/************************************************** _READER_THREAD_ ***************************************************************/
int sbuffer_remove(sbuffer_t *buffer) {
    
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_NO_DATA;
             // if not cleanup, lock data when reading

    pthread_mutex_lock(buffer->mutex_r_remove);
    dummy = buffer->head;
    buffer->head = buffer->head->next;
    pthread_mutex_unlock(buffer->mutex_r_remove);
    free(dummy);
    //µµsbuffer_node_t *dummy;
    //µµif (buffer == NULL) return SBUFFER_FAILURE;
    //µµif (buffer->head == NULL) return SBUFFER_NO_DATA;
    //µµ         // if not cleanup, lock data when reading
    
    //pthread_barrier_wait(buffer->barrier_r_nodesync);
     //fetch data to reader stack
    //SBUFF_DEBUG_PRINTF("Reader blocking on 'barrier_r_nodesync", " ");
    
    //if (sem_trywait(dummy->semaphore_r_processedcount) != 0) { // every reader has fetched the data
        //SBUFF_DEBUG_PRINTF("Reader removing node", " ");
    
    //pthread_mutex_lock(buffer->mutex_r_remove);
    //dummy = buffer->head;
    //*data = buffer->head->data;
    //buffer->head = buffer->head->next;
    //pthread_mutex_unlock(buffer->mutex_r_remove); */

    /* sem_destroy(dummy->semaphore_r_processedcount);
    pthread_barrier_destroy(dummy->barrier_r_nodesync);
    free(dummy->semaphore_r_processedcount);
    free(dummy->barrier_r_nodesync);
    free(dummy); 
    //} else {
    //    SBUFF_DEBUG_PRINTF("Reader continuing", " ");
    //    pthread_barrier_wait(dummy->barrier_r_nodesync);
    //}
    */
    return SBUFFER_SUCCESS;
}

/** Lock each reader on a sbuffer node and execute the reader callback functions.
 * 
*/

int rb_lock_execute(sbuffer_t* buffer) {
    if (buffer == NULL) return SBUFFER_FAILURE; // buffer must be initialized
    int no_r_callbacks = dpl_size(buffer->cbs_r);
    int check_exec_arr[no_r_callbacks];
    int node_lock;
    for (int m; m<no_r_callbacks; m++) {
        check_exec_arr[m] = 0;
    } 
    int count_r_callback_exec = no_r_callbacks; // logistics var
    sbuff_callback_t *dummy_callback_t;
    sbuffer_node_t *dummy_sbuff_node;
    
    dummy_sbuff_node = buffer->head; // start looping through the running buffer at the start of the sbuffer
    for (int idx_la= 0; idx_la<buffer->no_readers; idx_la++) { //search lock array for free buff node
        //DEBUG_PRINTF("Reader trying lock %d of nodelock_array",i);
        node_lock = pthread_mutex_trylock((buffer->nodelock_array)[dummy_sbuff_node->rb_index]);
        switch (node_lock) { //reader thread bound to node in ready sbuffer. Loop through loopback functions.
            case 0: 
                DEBUG_PRINTF("Reader locked on lock %d of nodelock_array", idx_la);
                while(count_r_callback_exec != 0) {                 // keep looping over the callbacks until all functions are executed by this thread
                    for (int j = 0; j < no_r_callbacks; j++) {      //
                        dummy_callback_t = dpl_get_element_at_index(buffer->cbs_r, j);
                        if (check_exec_arr[j] != 1){                //if function not yet executed
                            if(pthread_mutex_trylock(dummy_callback_t->exec_lock) == 0) { //if lock acquired
                                *(dummy_callback_t->arguments->data) = dummy_sbuff_node->data;
                                (dummy_callback_t->function)(dummy_callback_t->arguments); // execute reader callback
                                check_exec_arr[j] = 1;
                                count_r_callback_exec--;
                                count++;
                                pthread_mutex_unlock(dummy_callback_t->exec_lock);
                                GET_CALLBACK_FUNCTION_NAME(dummy_callback_t->function); // THESE TWO GO ALWAYS TOGETHER 
                                //DEBUG_PRINTF("Executed callback function: %s", f_name); 
                                FREE_CALLBACK_FUNCTION_NAME;                            // THESE TWO GO ALWAYS TOGETHER
                            }
                        }
                    }
                }

                DEBUG_PRINTF("Reader locked on lock %d blocking on barrier_r_readerssync after execution",idx_la);
                pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);
                /* if (idx_la == (buffer->no_readers - 1)) {
                    buffer->head = dummy_sbuff_node->next;
                    free(dummy_sbuff_node);
                    DEBUG_PRINTF("Removed sbuff at the end of running buffer (lockarray %d)", idx_la);
                } else {
                    free(dummy_sbuff_node);
                    DEBUG_PRINTF("Removed sbuff in middle of running buffer (lockarray %d)", idx_la);
                }
                pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);
 */
                pthread_mutex_unlock((buffer->nodelock_array)[dummy_sbuff_node->rb_index]);
                return SBUFFER_SUCCESS;
           
            case (EBUSY) : dummy_sbuff_node = dummy_sbuff_node->next; break;
            default      : ERROR_IF(1, "Design error during processing of data"); return SBUFFER_FAILURE;
        }
    }
    ERROR_IF(1, "Design error during processing of data");
    return SBUFFER_FAILURE;
}


void *reader_thread(void *buffer) {
    DEBUG_PRINTF("Reader thread started")
    pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_rw_startreaders);                                                         // Wait until all writers have buffered.
    int result = 0;
    DEBUG_PRINTF("Reader starts reading ")
    while(1) {
        if (pthread_rwlock_tryrdlock(((sbuffer_t*)buffer)->rwlock_rw_writing) == EBUSY) {               // writer still busy; lock not acquired.
            if (sem_trywait(((sbuffer_t*)buffer)->semaphore_rw_buffercount) == 0) {                     // buffer ready to process data
                // 1. reader 1 entered here
                // 2. rwlock opened


                DEBUG_PRINTF("Reader start looping on")
                rb_lock_execute((sbuffer_t*)buffer);
                //DEBUG_PRINTF("Reader removing sbuff_node")
                //sbuffer_remove(((sbuffer_t*)buffer)); 
                //3. reader Exits here
                continue;
                //rb_lock_execute(((sbuffer_t*)buffer));
                //DEBUG_PRINTF("Reader blocking on barrier_r_readerssync ")
                //pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);              
                //DEBUG_PRINTF("Reader lock 'r mutex_processing'")
                //pthread_mutex_lock(((sbuffer_t*)buffer)->mutex_r_processing);                
                //pthread_mutex_unlock(((sbuffer_t*)buffer)->mutex_r_processing);

            } else if (pthread_rwlock_tryrdlock(((sbuffer_t*)buffer)->rwlock_rw_writing) == EBUSY) {
                continue;
            } else {
                DEBUG_PRINTF("Reader unlock rwlock 'rwlock_rw_writing'")

                pthread_rwlock_unlock(((sbuffer_t*)buffer)->rwlock_rw_writing); // current thread jumps to else                                                  
            }

    // start cleanup function here
        } else {
            
            DEBUG_PRINTF("Reader unlock rwlock 'rwlock_rw_writing'")
            pthread_rwlock_unlock(((sbuffer_t*)buffer)->rwlock_rw_writing); 
            DEBUG_PRINTF("Reader exiting'")
            pthread_exit(0);                            // LOCK ACQUIRED! -> unlock for other reader threads. 
            DEBUG_PRINTF("Reader blocking on 'barrier_r_readerssync'")
            result = pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);                 // wait for other readers!
            
            if(result == 0) {                                                                //  close every reader thread except 1
                DEBUG_PRINTF("Reader going to sleep'")                             
                pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);                      //hold every reader but one
            } else {
                DEBUG_PRINTF("Reader still awake, determined %d remaining sbuffer nodes'", sbuffer_size(((sbuffer_t*)buffer)))
                ERROR_IF((sem_init(((sbuffer_t*)buffer)->semaphore_r_nodecount_cleanup,  0, sbuffer_size(((sbuffer_t*)buffer))) == -1), ERR_SEMAPHORE_INIT);
                pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync); //wake up other readers
            }            
            while (sem_trywait(((sbuffer_t*)buffer)->semaphore_r_nodecount_cleanup) == 0) {                   
                //sbuffer_remove(((sbuffer_t*)buffer));          // Remove node at the start of the sbuffer -> no interference with inserted nodes at the end
                //pthread_mutex_lock(((sbuffer_t*)buffer)->mutex_r_processing);
                //(_callback_func_)(_callback_arg_);
                //count++;
                //pthread_mutex_unlock(((sbuffer_t*)buffer)->mutex_r_processing);
                break;
                continue;
            }
            DEBUG_PRINTF("Reader exiting'")
            //DEBUG_PRINTF("Read %d data points from sbuffer", count); 
            DEBUG_PRINTF("writer delivered %d ready buffers", test); 
            pthread_exit(0);
        }
    }

    

}


/************************************************** _WRITER_THREAD_ ***************************************************************/
//TO DO: mimplement multiple writer possibility!

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    
    if (buffer == NULL) return SBUFFER_FAILURE; // buffer must be initialized
    sbuffer_node_t *dummy = (sbuffer_node_t*) malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL; 

    pthread_mutex_lock(buffer->mutex_w_calc_rb_index);
    
    if (buffer->write_la_index == buffer->no_readers) {
        buffer->write_la_index = 0;
        sem_post(buffer->semaphore_rw_buffercount);
        test++;
    }
    dummy->rb_index = buffer->write_la_index;
    buffer->write_la_index++;
    //DEBUG_PRINTF("inserted mutex array index %d", buffer->write_la_index);

    pthread_mutex_unlock(buffer->mutex_w_calc_rb_index);
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

void *writer_thread(void *buffer) {

    DEBUG_PRINTF("Writer starts ithn 'barrier_rw_startreaders' (address: %p)", &(((sbuffer_t*)buffer)->barrier_rw_startreaders));

    DEBUG_PRINTF("Writer thread started")    //sensor_data_t rx_data = {.id = 0, .value = 0, .ts = 0}; used  for multiple, dedicated writers
    int sbuff_count = 0;
    int wr_status = 0;
    sbuff_callback_t* cb = NULL;
    //Call user callback data processor
    DEBUG_PRINTF("Writer starts buffering.")
    cb = (sbuff_callback_t*)(dpl_get_element_at_index((((sbuffer_t*)buffer)->cbs_w), 0));
    ERROR_IF(cb == NULL, "No callback for sbuffer writer thread!");

    wr_status = (cb->function)((cb_args_t*)(cb->arguments));
    while (wr_status != DATA_PROCESS_ERROR) {
        sbuffer_insert(((sbuffer_t*)buffer), ((cb_args_t*)(cb->arguments))->data);
        sbuff_count++;
        if (sbuff_count == SBUFF_READER_THREADS + 1) break; // buffer until sbuffer contians SBUFF_READER_THREADS + 1 data nodes
        wr_status = (cb->function)(cb->arguments);
    }
    //ERROR_IF(wr_status == DATA_PROCESS_ERROR, "Error receving data!");
    DEBUG_PRINTF("Writer buffered %d sbuffer nodes.", sbuffer_size(((sbuffer_t*)buffer)))
    
    // TO DO:   implement mechanism so that multiple writers can lock the rwlock!
    DEBUG_PRINTF("Writer lock rwlock 'rwlock_rw_writing'")
    pthread_rwlock_wrlock(((sbuffer_t*)buffer)->rwlock_rw_writing); //SBUFFERED LOCK ON

    DEBUG_PRINTF("Writer blocks on 'barrier_rw_startreaders' (address: %p)", &(((sbuffer_t*)buffer)->barrier_rw_startreaders));
    pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_rw_startreaders); 

    DEBUG_PRINTF("Writer continue writing")
    wr_status = (cb->function)(cb->arguments);
    while (wr_status != DATA_PROCESS_ERROR) {
        sbuffer_insert(((sbuffer_t*)buffer), ((cb_args_t*)(cb->arguments))->data);
        sbuff_count++;
        sched_yield(); 
        wr_status = (cb->function)(cb->arguments);      
    }
    //ERROR_IF(wr_status == DATA_PROCESS_ERROR, "Error receving data!");
    DEBUG_PRINTF("Writer finished. Wrote %d sbuffer nodes", sbuff_count)
      
    // TO DO:   implement mechanism so that multiple writers can unlock the rwlock! (see above)
    DEBUG_PRINTF("Writer unlocking for buffered cleanup")
    pthread_rwlock_unlock(((sbuffer_t*)buffer)->rwlock_rw_writing);
    
    DEBUG_PRINTF("Writer exiting")
    pthread_exit(0);
    
}


/************************************************** _USER_FUNCTIONS_ ***************************************************************/
/** Adds the callback function to the reader/writer thread operating on the sbuffer.
 * TO_DO: Make adding callbacks thread safe.
 * \param buffer
 * \param function function pointer to the user defined callback function
 * \param args pointer to the callback arguments on the user stack. 
 *             Note that the user is resposible to not modify these while the threads operate!
 * \param type  
*/
int sbuffer_add_callback(sbuffer_t* buffer, cb_func function, cb_args_t* args, int rw) {
    if (buffer == NULL || function == NULL || args == NULL || (args)->data != NULL) return SBUFFER_FAILURE;
    
    

    
    //printf("sizeof(pthread_mutex_t): %d\n", sizeof(pthread_mutex_t));

    sbuff_callback_t elem = {.function = function, .arguments = args, .exec_lock = NULL};
    (&elem)->arguments->data = (sensor_data_t*) malloc(sizeof(sensor_data_t));
    ERROR_IF((&elem)->arguments->data == NULL, ERR_MALLOC("sensor_data"));

    (&elem)->exec_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    ERROR_IF((&elem)->exec_lock == NULL, ERR_MALLOC("mutex"));
    ERROR_IF(pthread_mutex_init((&elem)->exec_lock , NULL) == -1, ERR_MUTEX_INIT);

    switch (rw) {
        case SBUFFER_READER : dpl_insert_at_index(buffer->cbs_r, &elem, -1, true); break;
        case SBUFFER_WRITER : dpl_insert_at_index(buffer->cbs_w, &elem, -1, true); break;
        default             : ERROR_IF(1, "non-existing thread type! Only 'r' or 'w' allowed!");
    }
    free((&elem)->exec_lock);
    //DEBUG_PRINTF("size of cbs_w: %d", dpl_size(buffer->cbs_w));
    return(SBUFFER_SUCCESS);
}

/** Removes the callback function from the reader/writer thread operating on the sbuffer. Nothing happens if the function is
 *  not in the list.
 *  TO_DO: Make removing callbacks thread safe.
 * \param buffer
 * \param function function pointer to the user defined callback function
 * \param args pointer to the callback arguments on the user stack. 
 *             Note that the user is resposible to not modify these while the threads operate!
*/
int sbuffer_remove_callback(sbuffer_t* buffer, cb_func function, cb_args_t* args) {
    if (buffer == NULL || function == NULL) return SBUFFER_FAILURE;
    sbuff_callback_t elem = {.function = function, .arguments = args, .exec_lock = NULL};
    dpl_remove_element(buffer->cbs_r, &elem, true);
    dpl_remove_element(buffer->cbs_w, &elem, true);
    //printf("cbs_w callbacks remaining after remove: %d\n", dpl_size(buffer->cbs_w));
    //printf("cbs_r callbacks remaining after remove: %d\n", dpl_size(buffer->cbs_r));
    return(SBUFFER_SUCCESS);
}

/*******************************************************************************************************************************/
void * element_copy(void * element) {
    sbuff_callback_t* copy = malloc(sizeof (sbuff_callback_t));
    assert(copy != NULL);

    copy->function = ((sbuff_callback_t*)element)->function;
    copy->arguments = ((sbuff_callback_t*)element)->arguments;
    pthread_mutex_t* new_exec_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    ERROR_IF(new_exec_lock == NULL, ERR_MALLOC("mutex)"));
    ERROR_IF(pthread_mutex_init(new_exec_lock, NULL) == -1, ERR_MUTEX_INIT);

    copy->exec_lock = new_exec_lock;
    return (void *) copy;
}

void element_free(void ** element) {
    if (*element == NULL) return;
    else {
        pthread_mutex_destroy(((sbuff_callback_t*)(*element))->exec_lock);
        free(((sbuff_callback_t*)(*element))->exec_lock);
        free(((sbuff_callback_t*)(*element))->arguments->data);
        free(*element);
        *element = NULL;
    }
}

int element_compare(void * x, void * y) {
    DEBUG_PRINTF("pointer to cbw function: %p", ((sbuff_callback_t*)x)->function);
    return ((((sbuff_callback_t*)x)->function < ((sbuff_callback_t*)y)->function) ? -1 : 
            ((((sbuff_callback_t*)x)->function == ((sbuff_callback_t*)y)->function) &&
            ((((sbuff_callback_t*)x)->function == ((sbuff_callback_t*)y)->function))) ? 0 : 
            1);
}

void element_print(void* element) {
    char state = '0';
    if (pthread_mutex_trylock(((sbuff_callback_t*)element)->exec_lock) != 0) state = '1';  
    
    printf(" (callback_pointer$= %p, callback_arg_pointer= %p, mutex_state= %c (0 = open,1 = locked)\n",  
        ((sbuff_callback_t*)element)->function, 
        ((sbuff_callback_t*)element)->arguments, 
        state);
}

