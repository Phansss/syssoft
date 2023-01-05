/**
 * \author Pieter Hanssens
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "connmgr.h"
#include <wait.h>
#include "errmacros.h"
#include <assert.h>
#include <poll.h>
#include "lib/tcpsock.h"
#include <malloc.h>
/*********************************************************CORE FUNCTIONALITY*******************************************************************/

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
void* element_copy(void * element);                                             
void element_free(void ** element);   
int element_compare(void * x, void * y);

/**
 * Callback function to print the contents of an element of type my_socket_t inside a dplist.
 * \param element pointer to the element
 * \return Nothing. The content is printed on stdout
*/     
void element_print(void * element);  
void my_socket_print(my_socket_t* socket); 



void connmgr_listen(int port_number) {
    //open receiving file.
    FILE * sensor_data_recv = fopen("sensor_data_recv", "w+"); // w+ deletes the content of the file and creates it if it doesn't exist.
    setbuf(sensor_data_recv, NULL); // Unbuffered, data appears in file as soon as it is written
    // Variables for incoming new connection:
    tcpsock_t* passive_open_socket = NULL;
    tcpsock_t* new_socket_dummy = NULL;
    my_socket_t my_socket_element = {.socket = NULL, .lm = 0};
    //Variables to receive data from a client sensor:
    my_socket_t* client = NULL;
    sensor_data_t data = {.id = 0, .value = 0, .ts = 0};
    int bytes = 0;
    int result = 0;
    // Variables to control the while loop: Stop while loop if (timeout == true and close == true):
    bool timeout = false;   
    bool close = false;  

    // open a passive TCP connection
    if (tcp_passive_open(&passive_open_socket, port_number) != TCP_NO_ERROR) exit(EXIT_FAILURE);          

    //create server POLLFD of the passive connection -- TO DO: make static?
    pollfd_t server_pfd = { .fd = 0, .events = 0, .revents = 0};
    if (tcp_get_sd(passive_open_socket, &(server_pfd.fd)) != TCP_NO_ERROR) printf("socket not bound!\n");         
    server_pfd.events = POLLIN;  
  
    //create my_sockets_dplist
    dplist_t* my_sockets_dplist = NULL;
    my_sockets_dplist = dpl_create(element_copy, element_free, element_compare, element_print);
    
    //create pfds from server_pfd + my_sockets dplist
    pollfd_t *pfds = NULL;
    pfds = (pollfd_t *) calloc((dpl_size(my_sockets_dplist) + 1), sizeof(pollfd_t));
    pfds_recreate(&pfds, &server_pfd, my_sockets_dplist);      

    while ((!timeout) || (!close)) {
        //WHILE CONTROL
        if (timeout) close = true; // if timeout was not reset in previous loop, mark the while loop to close.
        timeout = true;

        POLL_ERROR(poll(pfds, dpl_size(my_sockets_dplist) + 1, TIMEOUT*1000));        // BLOCK until something happens, or timeout! -> puts current thread/process to sleep. TIMEOUT expressed in seconds but attr is in miliseconds.            
        
        // after POLL_TIMEOUT, check for events on server pfd
        if(pfds[0].revents & POLLIN) {                                                                               
            if (tcp_wait_for_connection(passive_open_socket, &new_socket_dummy) != TCP_NO_ERROR) exit(EXIT_FAILURE);            // new_socket_dummy is on heap.
            
            //create my_socket_element and insert in my_sockets_list.
            my_socket_element.socket = new_socket_dummy;
            time(&(my_socket_element.lm));
            dpl_insert_at_index(my_sockets_dplist, &my_socket_element, -1, true);

            //recreate pfds 
            pfds_recreate(&pfds, &server_pfd, my_sockets_dplist);

            //WHILE CONTROL
            timeout = false;    
            close = false;                      
        } 

        // after POLL_TIMEOUT, loop connected clients
        for (int i = 1; i<(dpl_size(my_sockets_dplist) + 1); i++) {       
            client = dpl_get_element_at_index(my_sockets_dplist, i-1);

            // check for events on current client
            if(pfds[i].revents & POLLIN) {
                //printf("Received sensor data!\n");
                 // read sensor ID
                bytes = sizeof(data.id);
                result = tcp_receive(client->socket, (void *) &data.id, &bytes);
                // read temperature
                bytes = sizeof(data.value);
                result = tcp_receive(client->socket, (void *) &data.value, &bytes);
                // read timestamp
                bytes = sizeof(data.ts);
                result = tcp_receive(client->socket, (void *) &data.ts, &bytes);
                
                if ((result == TCP_NO_ERROR) && bytes) {
                    printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int) data.ts);
                    fprintf(sensor_data_recv,"%hd %lf %ld\n",data.id,data.value, data.ts);

                    my_socket_update(client);       //update socket last modified time

                    //WHILE CONTROL
                    timeout = false; // something happened on one of the sensors -> reset timeout and close.
                    close = false;
                }
            }
             if ((time(NULL)-(client->lm)) >= TIMEOUT) { // time_t exprssed in seconds
                dpl_remove_element(my_sockets_dplist, client, true); // Remove and free element using element_free(). -> tcp connection closed.
                dpl_print(my_sockets_dplist);
                //recreate pfds 
                pfds_recreate(&pfds, &server_pfd, my_sockets_dplist);
            }
        }
    } 
    // close remaining connections
    dpl_free(&my_sockets_dplist, true); // destroy and close remaining sensor connections.
    tcp_close(&passive_open_socket);   // Close the passive TCP connection
    fclose(sensor_data_recv);       // close the data file
    free(pfds);
}

/** clean up the connmgr and free all used memory
 * 
*/
void connmgr_free() {
    return;
}

/**********************************************************SURROUNDING FUNCTIONALITY*********************************************************************/

int pfds_recreate(pollfd_t** pfds,pollfd_t* server_pfd, dplist_t* my_sockets_dplist) {
        
    (*pfds) = (pollfd_t *) realloc((*pfds),sizeof(pollfd_t)*(dpl_size(my_sockets_dplist) + 2));
    assert(*pfds != NULL);
    (*pfds)[0].fd = server_pfd->fd;             //
    (*pfds)[0].events = server_pfd->events;     // deepcopy the server pfd to the first pollfd in pfds
    (*pfds)[0].revents = server_pfd->revents;   //

    my_socket_t* dummy = NULL;
    for (int i = 0; i < dpl_size(my_sockets_dplist); i++) {     // for each socket in dplist, deepcopy the file descriptors into the pfds
        dummy = dpl_get_element_at_index(my_sockets_dplist, i);
        if (tcp_get_sd(dummy->socket , &((*pfds)[i+1].fd)) != TCP_NO_ERROR)  exit(1);
        (*pfds)[i+1].events = SENSOR_LISTEN;
        (*pfds)[i+1].revents = 0;
    }
    
    return 0;
}

/**********************************************************MY_SOCKET*********************************************************************/

void my_socket_print(my_socket_t* socket) {
    printf("socket with (tcpsock_t* socket = %p\n)", socket->socket);
    fflush(stdin);
}

/** Updates the lastmodified flag with the current system time. 
 *
 * \param socket the socket to update
*/
void my_socket_update(my_socket_t* socket) {
    time(&(socket->lm)); 
}

/*********************************************************CALLBACKS*******************************************************************/
void * element_copy(void * element) {
    int sd = 0;
     if (tcp_get_sd(((my_socket_t*)element)->socket, &sd) != TCP_NO_ERROR) exit(0);
    my_socket_t* copy = (my_socket_t*) calloc(1, sizeof(my_socket_t));
    assert(copy != NULL);
    copy->socket = ((my_socket_t*)element)->socket; //
    copy->lm = ((my_socket_t*)element)->lm; 
    return (void *) copy;
}

void element_free(void ** element) {
    if (*element == NULL) return;
    
    else {
        int sd = 0;
        if (tcp_get_sd((*((my_socket_t**)element))->socket, &sd)) exit(0); 
        printf("freeing socket %p with sd = %d\n", (*((my_socket_t**)element))->socket, sd);
        tcp_close(&((*((my_socket_t**)element))->socket));
        free((*(my_socket_t**)element)->socket);
        free(*element);
        *element = NULL;
    }
}

int element_compare(void * x, void * y) {
    return ((((my_socket_t*)x)->lm < ((my_socket_t*)y)->lm) ? -1 : // sort by sid
            ((((my_socket_t*)x)->lm == ((my_socket_t*)y)->lm) && ((((my_socket_t*)x)->socket == ((my_socket_t*)y)->socket))) ? 0 : //only equal when socket == socket & lm == lm
             1);
}

void element_print(void* element) {
    char buff[25];
    struct tm* timeinfo;
    timeinfo = localtime(&(((my_socket_t*)element)->lm));
    strftime(buff, 25, "%c", timeinfo);
    int sd;
    tcp_get_sd((tcpsock_t*)((my_socket_t*)element)->socket , &sd);
    printf(" (socket_addr=%p, socket_fd=%d, lm = %s)\n",  
     (tcpsock_t*)((my_socket_t*)element)->socket, sd, buff);
}





