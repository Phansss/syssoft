#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "main_debug.h"
#include "callbacks/callbacks.h"


#define DEBUG
#define DEB_EXIT

#include <pthread.h>
#include <semaphore.h>
#include <inttypes.h>

#include "errmacros.h"
#include "lib/dplist.h"
#include <connmgr.h>
#include "lib/tcpsock.h"



#define INIT_PFDA 5   //Initial size of polling array
#define SERVER_LISTEN POLLIN & POLLHUP
#define SENSOR_LISTEN POLLIN
#define POLL_TIMEOUT -1 //No-event Timeout (in miliseconds). Event defined as adding a sensor or receiving sensor data.

#ifndef TIMEOUT
//#error Please define TIMEOUT!
#define TIMEOUT 5
#endif

/** Structure to store connected sensor listener logistics.
 * 
*/
typedef struct {
    tcpsock_t* socket;                        // 
    time_t lm;                               // timestamp last modified
} my_socket_t;

typedef struct pollfd pollfd_t;

int pfds_recreate(pollfd_t** pfds, pollfd_t* server_pfd, dplist_t* my_sockets_dplist);
/** Updates the lastmodified flag with the current system time. 
 *
 * \param socket the socket to update
*/
void my_socket_update(my_socket_t* socket);
/** Copies the given element.
 * \param element pointer to the element to be copied
 * \return copy pointer to the copied selement.
*/ 

#define WRITER_DATA_IN_EMPTY -1

typedef size_t (*Freadfunc)(void*, size_t, size_t, FILE*);


int main() {
  int child = fork();
  SYSCALL_ERROR(child);

  if (child == 0) {
    

  } 
  else {
    pthread_t writer;
    pthread_t reader1;
    pthread_t reader2;
    sbuffer_t* buffer;

    //initialize sbuffer
    if (sbuffer_init(&buffer, SBUFF_READER_THREADS, SBUFF_WRITER_THREADS) != SBUFFER_SUCCESS) exit(1);

    //initialize resources
    read_file_t* read_file_args;
    write_file_t* write_file_args;
    read_file_init(&write_file_args);
    write_file_init(&write_file_args);

    sbuffer_add_callback(buffer, read_file, (cb_args_t*)read_file_args, SBUFFER_WRITER);
    sbuffer_add_callback(buffer, write_file, (cb_args_t*)write_file_args, SBUFFER_READER);

    PTH_create(&writer, NULL, writer_thread, buffer); 
    PTH_create(&reader1, NULL, reader_thread, buffer); 
    PTH_create(&reader2, NULL, reader_thread, buffer); 
  
    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    // Close resources
    read_file_cleanup(&read_file_args);
    write_file_cleanup(&write_file_args);
  
    sbuffer_free(&buffer);
  }  
  return 0;
}

