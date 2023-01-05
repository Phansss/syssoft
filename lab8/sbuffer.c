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


#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#define NO_OF_READERS 2

extern sem_t semaphore;
extern pthread_barrier_t barrier;
extern pthread_barrier_t read_sync_finalize_1;
extern pthread_barrier_t read_sync_finalize_2;
extern pthread_rwlock_t rwlock;
extern pthread_mutex_t mutex;
extern pthread_mutex_t write_bin_out;
extern pthread_rwlock_t sbuffered_cleanup;
extern sbuffer_t* buffer;
extern pthread_cond_t writing_stopped;

int writing;
int count;

pthread_t writer = 0;
pthread_t reader1 = 1;
pthread_t reader2 = 2;
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
    
    switch (cleanup){
        case 0:                                                 // if not cleanup, lock data when reading
            //printf("Thread %d aquiring mutex lock\n", x);
            pthread_mutex_lock(&mutex);
            //printf("Thread %d aquired mutex lock\n", x);
            *data = buffer->head->data;
            dummy = buffer->head;
            buffer->head = buffer->head->next;
            //printf("Thread %d unlocking mutex lock\n", x);
            pthread_mutex_unlock(&mutex);
            break;
        case 1:                                                 // if cleanup, no mutex needed, only one reader thread.
            *data = buffer->head->data;
            dummy = buffer->head;
            buffer->head = buffer->head->next;
        default:
            DEBUG_PRINTF("Error during sbuffer_remove()");
    }

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

    // WRITE LOCK
    //printf("Aquiring WRITE lock\n");
    //printf("WRITE lock aquired\n");
    if (buffer->tail == NULL) // buffer empty 
    {   
        buffer->head = buffer->tail = dummy;

    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    //end write lock
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




// writer thread
void *writer_thread(void *arg) {
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0};
    int sbuff_count = 0;
    int sem_val=0;
    writing = 1;


    //pthread_rwlock_wrlock(&rwlock);
    while (fread(&(data.id), sizeof(sensor_id_t), 1, ((thread_attr_t*)arg)->binary_file)!= 0) {
        printf("inserting sbuff\n");
        fread(&(data.value), sizeof(sensor_value_t), 1, ((thread_attr_t*)arg)->binary_file);
        fread(&(data.ts), sizeof(sensor_ts_t), 1, ((thread_attr_t*)arg)->binary_file);
        sbuffer_insert(((thread_attr_t*)arg)->sbuffer, &data);
        sbuff_count++;
        if (sbuff_count == NO_OF_READERS + 1) break; // buffer until sbuffer contians NO_OF_READERS + 1 data nodes

    }
    pthread_rwlock_wrlock(&sbuffered_cleanup); //SBUFFERED LOCK ON
    printf("Sbuffer buffered %d data elements. Starting to read..\n", sbuffer_size(((thread_attr_t*)arg)->sbuffer));    

    if(sem_init(&semaphore, 0, 0) == -1) DEBUG_PRINTF("Pthread semaphore init error\n"); // create semaphore
    thread_attr_t reader_attr = {.binary_file = binary_file_out, .sbuffer = buffer}; // Resources for reader thread

    PTH_create(&reader1, NULL, reader_thread, &reader_attr); // Start reading (tries sem and yields immediately (sem ==0))
    PTH_create(&reader2, NULL, reader_thread, &reader_attr); // Start reading (tries sem and yields immediately (sem ==0))
    
    printf("Continue writing after buffering\n");
    while (fread(&(data.id), sizeof(sensor_id_t), 1, ((thread_attr_t*)arg)->binary_file)!= 0) {
        fread(&(data.value), sizeof(sensor_value_t), 1, ((thread_attr_t*)arg)->binary_file);
        fread(&(data.ts), sizeof(sensor_ts_t), 1, ((thread_attr_t*)arg)->binary_file); 
        
        //printf("inserting buff\n");
        sbuffer_insert(((thread_attr_t*)arg)->sbuffer, &data);
        //release semaphore
        sem_post(&semaphore);
        sched_yield();        
    }
    
    sem_getvalue(&semaphore, &sem_val);
    //printf("sbuffer size (sem value): %d\n", sem_val);
    
    printf("WRITER THREAD: unlocked and waiting to exit (%d remaining sems)\n", sem_val);    
    pthread_rwlock_unlock(&sbuffered_cleanup);               //SBUFFERED LOCK OFF
    //pthread_rwlock_unlock(&rwlock);
    int remaining_sbuff;
    remaining_sbuff = sbuffer_size(((thread_attr_t*)arg)->sbuffer);
    printf("WRITER THREAD: Remaining sbuffer elements: %d\n", remaining_sbuff);
   
    usleep(500);
    pthread_barrier_wait(&barrier);
    printf("writer exiting\n");
    pthread_exit(0);
}

// reader thread
void *reader_thread(void *arg) {
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0}; // data is thread safe variable
    //pid_t x = syscall(__NR_gettid);
    int result = 0;
    //int sem_val;
    int remaining_sbuff;
    while(1) {
        if (pthread_rwlock_tryrdlock(&sbuffered_cleanup) == EBUSY) {               // writer still busy; lock not acquired.
            if (sem_trywait(&semaphore) == 0) {                                    // block until resources are available in the sbuffer
                sbuffer_remove(((thread_attr_t*)arg)->sbuffer, &data, 0);       // Remove node at the start of the sbuffer -> no interference with inserted nodes at the end
                //printf("(%d): sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", x, data.id, data.value, (long int) data.ts);
                //pthread_mutex_lock(&write_bin_out);
                fprintf(((thread_attr_t*)arg)->binary_file,"%hd %lf %ld\n",data.id,data.value, data.ts);
                //printf("READER THREAD %d has written data to bin out\n", x);
                count++;
                //pthread_mutex_unlock(&write_bin_out);
                continue;
            } else if (pthread_rwlock_tryrdlock(&sbuffered_cleanup) == EBUSY) {
                continue;
            } else {
                pthread_rwlock_unlock(&sbuffered_cleanup);
                
                //printf("THREAD %d blocking on read_sync_finalize_1\n", x);
                //result = pthread_barrier_wait(&read_sync_finalize_1);                      // go to else via lock when other readers have caught up
                //printf("THREAD %d continuing on read_sync_finalize_1\n", x);
                                                                    
            }
        } else {
            pthread_rwlock_unlock(&sbuffered_cleanup);   
            //printf("THREAD %d blocking on read_sync_finalize_2\n", x);                        // LOCK ACQUIRED! -> unlock for other reader threads.
            result = pthread_barrier_wait(&read_sync_finalize_2);                 // wait for other readers!
            //printf("THREAD %d continuing on read_sync_finalize_2\n", x);
            if(result == 0) {    
                //printf("THREAD %d blocked on main barrier\n", x);                                               //  close every reader thread except 1.
                pthread_barrier_wait(&barrier);
                //printf("THREAD %d shutting down\n", x);                                  //  signal the main barrier that this thread is about to close.
                pthread_exit(0);
            }
            remaining_sbuff = sbuffer_size(((thread_attr_t*)arg)->sbuffer);
            //printf("THREAD %d: Remaining sbuffer elements: %d\n",x, remaining_sbuff);
            //printf("THREAD %d blocked on main barrier\n", x);
            while(remaining_sbuff > 0) {
                remaining_sbuff--;
                sbuffer_remove(((thread_attr_t*)arg)->sbuffer, &data, 1);
                fprintf(((thread_attr_t*)arg)->binary_file,"%hd %lf %ld\n",data.id,data.value, data.ts);
                count++;
            } 
            pthread_barrier_wait(&barrier); 
            //remaining_sbuff = sbuffer_size(((thread_attr_t*)arg)->sbuffer);
            printf("written %d times\n", count);
            //printf("THREAD %d: Remaining sbuffer elements: %d\n",x, remaining_sbuff);
            //printf("THREAD %d shutting down\n", x); 
            pthread_exit(0);
            
            
            
            //printf("result_PTHREAD_...= %d\n", PTHREAD_BARRIER_SERIAL_THREAD);
            //printf("result= %d\n", result);
        }
        
    }
    /* printf("THREAD %d: Remaining sbuffer elements: %d\n",x, remaining_sbuff);
        //sem_getvalue(&semaphore, &sem_val);
        //printf("Thread %d sbuffer size (sem value): %d\n",x, sem_val);
        //printf("Thread %d synced and waiting to exit\n", x);
        printf("THREAD %d: blocked\n",x);
        pthread_barrier_wait(&barrier);
        pthread_exit(0); */
}


