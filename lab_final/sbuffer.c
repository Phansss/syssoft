/**
 * \author Pieter Hanssens
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include "debugger.h"
#include "errhandler.h"

#include "sbuffer.h"
#include "lib/dplist.h"
#include <pthread.h>
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

int count;
void *reader_cleanup(sbuffer_t* buffer);
void * my_callback_copy(void * element);
void my_callback_free(void ** element);
int my_callback_compare(void * x, void * y);
void my_callback_print(void * element);

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
    pthread_barrier_t* barrier_r_cleanupsync;
    
    sem_t* semaphore_rw_buffercount; // initialized in writer
    sem_t* semaphore_count_nodes_ready; // initialized at start of cleanup
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
    ((*buffer)->cbs_r) = dpl_create(my_callback_copy, my_callback_free, my_callback_compare, my_callback_print);
    ((*buffer)->cbs_w) = dpl_create(my_callback_copy, my_callback_free, my_callback_compare, my_callback_print);  
    
    (*buffer)->rwlock_rw_writing = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t)); 
    ERROR_IF((*buffer)->rwlock_rw_writing == NULL, ERR_MALLOC("rwlock"));
    ERROR_IF((pthread_rwlock_init((*buffer)->rwlock_rw_writing, NULL) == -1), ERR_RWLOCK_INIT);
    
    (*buffer)->semaphore_rw_buffercount = (sem_t*) malloc(sizeof(sem_t));
    ERROR_IF((*buffer)->semaphore_rw_buffercount == NULL, ERR_MALLOC("semaphore"));
    ERROR_IF((sem_init((*buffer)->semaphore_rw_buffercount,  0, 0) == -1), ERR_SEMAPHORE_INIT);
    
    (*buffer)->semaphore_count_nodes_ready = (sem_t*) malloc(sizeof(sem_t));
    ERROR_IF((*buffer)->semaphore_count_nodes_ready == NULL, ERR_MALLOC("semaphore")); // initialization at cleanup
    ERROR_IF((sem_init((*buffer)->semaphore_count_nodes_ready,  0, 0) == -1), ERR_SEMAPHORE_INIT);

    (*buffer)->barrier_r_readerssync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_r_readerssync == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_r_readerssync, NULL, no_readers) == -1), ERR_BARRIER_INIT);
    
    
    (*buffer)->barrier_r_nodesync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_r_nodesync == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_r_nodesync, NULL, no_readers) == -1), ERR_BARRIER_INIT);

    (*buffer)->barrier_rw_startreaders = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_rw_startreaders == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_rw_startreaders, NULL, no_readers + no_writers) == -1), ERR_BARRIER_INIT);
    
    (*buffer)->barrier_r_cleanupsync = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t));
    ERROR_IF((*buffer)->barrier_r_cleanupsync == NULL, ERR_MALLOC("barrier"));
    ERROR_IF((pthread_barrier_init((*buffer)->barrier_r_cleanupsync, NULL, no_readers) == -1), ERR_BARRIER_INIT);
    
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
    PRINTF_SBUFFER("Initialized 'barrier_rw_startreaders' at address: %p", &(((sbuffer_t*)buffer)->barrier_rw_startreaders));
    
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    pthread_rwlock_destroy((*buffer)->rwlock_rw_writing);
    sem_destroy((*buffer)->semaphore_rw_buffercount);
    sem_destroy((*buffer)->semaphore_count_nodes_ready);
    pthread_barrier_destroy((*buffer)->barrier_r_readerssync);
    pthread_barrier_destroy((*buffer)->barrier_r_nodesync);
    pthread_barrier_destroy((*buffer)->barrier_rw_startreaders);
    pthread_barrier_destroy((*buffer)->barrier_r_cleanupsync);
    pthread_mutex_destroy((*buffer)->mutex_r_remove);
    pthread_mutex_destroy((*buffer)->mutex_r_processing);
    pthread_mutex_destroy((*buffer)->mutex_w_calc_rb_index);
    for (int i=0; i<(*buffer)->no_readers; i++) {
        pthread_mutex_destroy(((*buffer)->nodelock_array)[i]);
        free(((*buffer)->nodelock_array)[i]);
    }
    dpl_free(&((*buffer)->cbs_r), true);
    dpl_free(&((*buffer)->cbs_w), true);
    free((*buffer)->rwlock_rw_writing);
    free((*buffer)->semaphore_rw_buffercount);
    free((*buffer)->semaphore_count_nodes_ready);
    free((*buffer)->barrier_r_readerssync);
    free((*buffer)->barrier_r_nodesync);
    free((*buffer)->barrier_rw_startreaders);
    free((*buffer)->barrier_r_cleanupsync);
    free((*buffer)->mutex_r_remove);
    free((*buffer)->mutex_r_processing);
    free((*buffer)->mutex_w_calc_rb_index);
    free((*buffer)->nodelock_array);
    
    while ((*buffer)->head) {
        sbuffer_node_t *dummy;
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
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
        
    pthread_mutex_lock(buffer->mutex_r_remove);
    dummy = buffer->head;
    buffer->head = buffer->head->next;
    pthread_mutex_unlock(buffer->mutex_r_remove);
    free(dummy);
    
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
        //PRINTF_SBUFFER("Reader trying lock %d of nodelock_array",i);
        node_lock = pthread_mutex_trylock((buffer->nodelock_array)[dummy_sbuff_node->rb_index]);
        switch (node_lock) { //reader thread bound to node in ready sbuffer. Loop through loopback functions.
            case 0:
                //PRINTF_SBUFFER("Reader locked on lock %d of nodelock_array", idx_la);
                while(count_r_callback_exec != 0) {                 // keep looping over the callbacks until all functions are executed by this thread
                    for (int j = 0; j < no_r_callbacks; j++) {      //
                        dummy_callback_t = dpl_get_element_at_index(buffer->cbs_r, j);
                        if (check_exec_arr[j] != 1){                //if function not yet executed
                            if(pthread_mutex_trylock(dummy_callback_t->exec_lock) == 0) { //if lock acquired
                                *(dummy_callback_t->arguments->data_buffer->head) = dummy_sbuff_node->data;
                                (dummy_callback_t->function)(dummy_callback_t->arguments); // execute reader callback
                                check_exec_arr[j] = 1;
                                count_r_callback_exec--;
                                //count++;
                                pthread_mutex_unlock(dummy_callback_t->exec_lock);
                                //GET_CALLBACK_FUNCTION_NAME(dummy_callback_t->function); // THESE TWO GO ALWAYS TOGETHER 
                                //PRINTF_SBUFFER("Reader executed callback function: %s", f_name); 
                                //FREE_CALLBACK_FUNCTION_NAME;                            // THESE TWO GO ALWAYS TOGETHER
                            }
                        }
                    }
                }
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
    
    PRINTF_SBUFFER("Reader thread started");
    pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_rw_startreaders);                                                        // Wait until all writers have buffered.
    int result;
    PRINTF_SBUFFER("Reader starts reading ");
    //int sv = 0;
    sbuffer_node_t* sbuff_dummy;
    sbuffer_node_t* sbuff_dummy_prev;
    while(1) {
        //if (pthread_rwlock_tryrdlock(((sbuffer_t*)buffer)->rwlock_rw_writing) == EBUSY) {
            //PRINTF_SBUFFER("Address of ((sbuffer_t*)buffer)->semaphore_count_nodes_ready :%p", ((sbuffer_t*)buffer)->semaphore_count_nodes_ready)               // writer still busy; lock not acquired.
            if (sem_trywait(((sbuffer_t*)buffer)->semaphore_count_nodes_ready) == 0) {   
                //PRINTF_SBUFFER("rb_lock_execute"); 
                rb_lock_execute((sbuffer_t*)buffer);
                //PRINTF_SBUFFER("Reader blocking on barrier 'barier_r_readerssync"); 
                result = pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);
                if (result != PTHREAD_BARRIER_SERIAL_THREAD) sched_yield();
                else { sbuff_dummy = ((sbuffer_t*)buffer)->head; //look for address of last node in running buffer
                    //end loop when sbuff dummy is at start of new buffer
                    for (int i = 0; i < ((sbuffer_t*)buffer)->no_readers; i++) {
                        sbuff_dummy_prev = sbuff_dummy;
                        sbuff_dummy = sbuff_dummy->next;
                        free(sbuff_dummy_prev);
                    }
                    //PRINTF_SBUFFER("Address of ((sbuffer_t*)buffer)->head :%p", &(((sbuffer_t*)buffer)->head))   
                    ((sbuffer_t*)buffer)->head = sbuff_dummy;
                }
            //PRINTF_SBUFFER("Read %d data points from sbuffer", count); 
            //PRINTF_SBUFFER("Waiting for removal of previous buffer")
            pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_r_readerssync);
            } else{
                if (pthread_rwlock_tryrdlock(((sbuffer_t*)buffer)->rwlock_rw_writing) != EBUSY) {
                    pthread_rwlock_unlock(((sbuffer_t*)buffer)->rwlock_rw_writing);
                    reader_cleanup(buffer);
                }
            }
        } 
    }     


void *reader_cleanup(sbuffer_t* buffer) {
    PRINTF_SBUFFER("Enter reader cleanup");
    pthread_exit(0);
}
                
/************************************************** _WRITER_THREAD_ ***************************************************************/
//TO DO: mimplement multiple writer possibility!



int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data) {
    
    if (buffer == NULL) return SBUFFER_FAILURE; // buffer must be initialized
    sbuffer_node_t *dummy = (sbuffer_node_t*) malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL; 

    if (buffer->write_la_index == buffer->no_readers) {
        buffer->write_la_index = 0;
        sem_post(buffer->semaphore_rw_buffercount);
        //test++;
    }
    dummy->rb_index = buffer->write_la_index;
    buffer->write_la_index++;

    if (buffer->tail == NULL) // buffer empty, first read
    {   
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    sem_post(buffer->semaphore_count_nodes_ready);
    return SBUFFER_SUCCESS;
}
#define _sbuff_data_buffer_head_ ((cb_args_t*)(cb->arguments))->data_buffer->head

void *writer_thread(void *buffer) {

    PRINTF_SBUFFER("Writer thread started");    //sensor_data_t rx_data = {.id = 0, .value = 0, .ts = 0}; used  for multiple, dedicated writers
    int sbuff_count = 0;
    sbuff_callback_t* cb = NULL;

    //initialize callback & accompanying data_buffer
    cb = (sbuff_callback_t*)(dpl_get_element_at_index((((sbuffer_t*)buffer)->cbs_w), 0));
    //ERROR_IF(cb == NULL, "No callback for sbuffer writer thread!"); -> cppcheck --> cb == NULL: redundant 

    
    // callback execution
    PRINTF_SBUFFER("Writer execute callback with");
    while ((cb->function)((cb_args_t*)(cb->arguments)) != DATA_PROCESS_ERROR) {
        for (int i = 1; i <= ((cb_args_t*)(cb->arguments))->data_buffer->count_received; i++) {
            PRINTF_SBUFFER("Writer inserting data in sbuffer: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld", 
                            ((_sbuff_data_buffer_head_)[i-1]).id, 
                            ((_sbuff_data_buffer_head_)[i-1]).value, 
                            ((long int)((_sbuff_data_buffer_head_)[i-1]).ts));
            sbuffer_insert(((sbuffer_t*)buffer), &((_sbuff_data_buffer_head_)[i-1]));
            sbuff_count++;
            if (sbuff_count == (SBUFF_READER_THREADS*SBUFF_WRITE_BUFFER)) break; // buffer until sbuffer contians SBUFF_READER_THREADS + 1 data nodes
        }
        if (sbuff_count == (SBUFF_READER_THREADS*SBUFF_WRITE_BUFFER)) break;
    }


    //ERROR_IF(wr_status == DATA_PROCESS_ERROR, "Error receving data!");
    PRINTF_SBUFFER("Writer buffered %d sbuffer nodes.", sbuffer_size(((sbuffer_t*)buffer)));
    
    // TO DO:   implement mechanism so that multiple writers can lock the rwlock!
    PRINTF_SBUFFER("Writer lock rwlock 'rwlock_rw_writing'");
    pthread_rwlock_wrlock(((sbuffer_t*)buffer)->rwlock_rw_writing); //SBUFFERED LOCK ON

    PRINTF_SBUFFER("Writer blocks on 'barrier_rw_startreaders' (address: %p)", &(((sbuffer_t*)buffer)->barrier_rw_startreaders));
    pthread_barrier_wait(((sbuffer_t*)buffer)->barrier_rw_startreaders); 

    PRINTF_SBUFFER("Writer continue writing");
    while ((cb->function)((cb_args_t*)(cb->arguments)) != DATA_PROCESS_ERROR) {
        for (int i = 1; i <= ((cb_args_t*)(cb->arguments))->data_buffer->count_received; i++) {
            PRINTF_SBUFFER("Writer inserting data in sbuffer: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld", 
                            ((_sbuff_data_buffer_head_)[i-1]).id, 
                            ((_sbuff_data_buffer_head_)[i-1]).value, 
                            ((long int)((_sbuff_data_buffer_head_)[i-1]).ts));
            sbuffer_insert(((sbuffer_t*)buffer), &((_sbuff_data_buffer_head_)[i-1]));
            sbuff_count++;
        }
        if (sbuff_count == (SBUFF_READER_THREADS*SBUFF_WRITE_BUFFER)) break;
    }
    PRINTF_SBUFFER("Writer finished. Wrote %d sbuffer nodes", sbuff_count);
      
    // TO DO:   implement mechanism so that multiple writers can unlock the rwlock! (see above)
    PRINTF_SBUFFER("Writer unlocking for buffered cleanup");
    pthread_rwlock_unlock(((sbuffer_t*)buffer)->rwlock_rw_writing);
    
    PRINTF_SBUFFER("Writer exiting, Free this callback using sbuffer_remove_callback() or \
                    sbuffer_free()!");

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
    if (buffer == NULL || function == NULL || args == NULL || (args)->data_buffer != NULL) return SBUFFER_FAILURE;
    
    //printf("sizeof(pthread_mutex_t): %d\n", sizeof(pthread_mutex_t));
    
    sbuff_callback_t elem = {.function = function, .arguments = args, .exec_lock = NULL};
    ((&elem)->arguments)->data_buffer = calloc(1, sizeof(data_buffer_t));
    ERROR_IF(((&elem)->arguments)->data_buffer == NULL, ERR_MALLOC("data_buffer_t in writer thread"));
    ((&elem)->arguments)->data_buffer->size = DATA_BUFFER_SIZE;
    ((&elem)->arguments)->data_buffer->count_received = 0;    
    (((&elem)->arguments)->data_buffer->head) = calloc((((&elem)->arguments)->data_buffer->size)+1, sizeof(sensor_data_t));
    
    ERROR_IF(((&elem)->arguments)->data_buffer->head == NULL, ERR_MALLOC("sensor_data* x SIZE_DATA_BUFFER in writer thread"));
    (&elem)->exec_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    ERROR_IF((&elem)->exec_lock == NULL, ERR_MALLOC("mutex"));
    ERROR_IF(pthread_mutex_init((&elem)->exec_lock , NULL) == -1, ERR_MUTEX_INIT);
    switch (rw) {
        case SBUFFER_READER : dpl_insert_at_index(buffer->cbs_r, &elem, -1, true); break;
        case SBUFFER_WRITER : dpl_insert_at_index(buffer->cbs_w, &elem, -1, true);break;
        default             : ERROR_IF(1, "non-existing thread type! Only 'r' or 'w' allowed!");
    }
    free((&elem)->exec_lock);
    PRINTF_SBUFFER("size of cbs_r: %d", dpl_size(buffer->cbs_r));
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
void * my_callback_copy(void * element) {
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

void my_callback_free(void ** element) {
    if (*element == NULL) return;
    else {
        pthread_mutex_destroy(((sbuff_callback_t*)(*element))->exec_lock);
        free(((sbuff_callback_t*)(*element))->exec_lock);
        //PRINTF_SBUFFER("pointer to buffer: %p", ((sbuff_callback_t*)(*element))->arguments->data_buffer)
        //free(((sbuff_callback_t*)(*element))->arguments->data_buffer->head);
        //free(((sbuff_callback_t*)(*element))->arguments->data_buffer);
        ((sbuff_callback_t*)(*element))->arguments->data_buffer = NULL;
        free(*element);
        *element = NULL;
    }
}

int my_callback_compare(void * x, void * y) {
    //PRINTF_SBUFFER("pointer to cbw function: %p", ((sbuff_callback_t*)x)->function);
    return ((((sbuff_callback_t*)x)->function < ((sbuff_callback_t*)y)->function) ? -1 : 
            ((((sbuff_callback_t*)x)->function == ((sbuff_callback_t*)y)->function) &&
            ((((sbuff_callback_t*)x)->arguments == ((sbuff_callback_t*)y)->arguments))) ? 0 : 
            1);
}

void my_callback_print(void* element) {
    char state = '0';
    if (pthread_mutex_trylock(((sbuff_callback_t*)element)->exec_lock) != 0) state = '1';  
    
    printf(" (callback_pointer$= %p, callback_arg_pointer= %p, mutex_state= %c (0 = open,1 = locked)\n",  
        ((sbuff_callback_t*)element)->function, 
        ((sbuff_callback_t*)element)->arguments, 
        state);
}

