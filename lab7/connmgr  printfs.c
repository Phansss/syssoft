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

void connmgr_listen(int port_number) {
    printf("***********************************************************************************\n");
    printf("Listening...\n");
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
    printf("-----------------------------------\n");

    while ((!timeout) || (!close)) {
        //WHILE CONTROL
        if (timeout) close = true; // if timeout was not reset in previous loop, mark the while loop to close.
        timeout = true;

        //printf("start polling\n");
        POLL_ERROR(poll(pfds, dpl_size(my_sockets_dplist) + 1, TIMEOUT*1000));        // BLOCK until something happens, or timeout! -> puts current thread/process to sleep. TIMEOUT expressed in seconds but attr is in miliseconds.
        //printf("finish polling\n");               
        
        // after POLL_TIMEOUT, check for events on server pfd
        if(pfds[0].revents & POLLIN) {                                                                               
            //printf("detected new sensor!\n"); 
            if (tcp_wait_for_connection(passive_open_socket, &new_socket_dummy) != TCP_NO_ERROR) exit(EXIT_FAILURE);            // new_socket_dummy is on heap.
            
            //printf("creating new my_socket_element\n");
            //create my_socket_element and insert in my_sockets_list.
            my_socket_element.socket = new_socket_dummy;
            time(&(my_socket_element.lm));
            //my_socket_create(&my_socket_element, new_socket_dummy);
            
            
            //printf("incoming TCP_SOCK pointer: %p\n", new_socket_dummy);
            //printf("MCREATED MY_SOCKET : ");
            //my_socket_print(my_socket_element);
            printf("INSERTING NEW SOCKET\n");
            dpl_insert_at_index(my_sockets_dplist, &my_socket_element, -1, true);
            //printf("destroying new my_socket_element on pointer %p\n", my_socket_element);
            //my_socket_destroy(&my_socket_element, false); // Deep copy created BUT do not disconnect the socket (deep copy does not include tcpsock_t deepcopy!)    
            

            //printf("current my_sock_list: ");
            // dpl_print(my_sockets_dplist);
            printf("-----------------------------------\n");
            
            //recreate pfds 
            printf("pfds: %p\n", pfds);
           
            pfds_recreate(&pfds, &server_pfd, my_sockets_dplist);
            
            printf("connected %d sensors\n", dpl_size(my_sockets_dplist));

            //WHILE CONTROL
            timeout = false;    
            close = false;                      
        } 

        // after POLL_TIMEOUT, loop connected clients
        for (int i = 1; i<(dpl_size(my_sockets_dplist) + 1); i++) {       
            client = dpl_get_element_at_index(my_sockets_dplist, i-1);

            //printf("Start checking for events on sensor\n");
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

                my_socket_update(client);       //update socket last modified time
                
                //WHILE CONTROL
                timeout = false; // something happened on one of the sensors -> reset timeout and close.
                close = false;
                }
            }

            
            //printf("Finish checking for events on sensor\n");
            
            //printf("Start checking for timeout\n");
            // check for timeout on current client
            if ((time(NULL)-(client->lm)) >= TIMEOUT) { // time_t exprssed in seconds
                printf("Client hangup!\n");
                dpl_remove_element(my_sockets_dplist, client, true); // Remove and free element using element_free(). -> tcp connection closed.
                dpl_print(my_sockets_dplist);
                //recreate pfds 
                pfds_recreate(&pfds, &server_pfd, my_sockets_dplist);
            }
            //printf("Finish checking for timeout\n");

        }
        if (timeout && close) {
            printf("closing loop.\n");
        }     
    } 
    // close remaining connections
    dpl_free(&my_sockets_dplist, true); // destroy and close remaining sensor connections.
    tcp_close(&passive_open_socket);   // Close the passive TCP connection
    free(pfds);
    printf("Stopped listening due to timeout\n");
}


/**********************************************************SURROUNDING FUNCTIONALITY*********************************************************************/

int pfds_recreate(pollfd_t** pfds,pollfd_t* server_pfd, dplist_t* my_sockets_dplist) {
    
    //printf("\nRecreating pfds on pointer %p\n\n", *pfds);
    //for (int i = 0; i < dpl_size(my_sockets_dplist); i++) { 
    //    printf("            pfds[%d] = (fd=%d, events=%d, revents = %d)\n", i, (*pfds)[i].fd, (*pfds)[i].events, (*pfds)[i].revents);
    //}
    //printf("Requesting %ld bytes through malloc\n", sizeof(pollfd_t)*(dpl_size(my_sockets_dplist) + 1));
    //printf("pfds pointer: %p", *pfds);
    //printf("\n\nsleeping\n\n");
    
    (*pfds) = (pollfd_t *) realloc((*pfds),sizeof(pollfd_t)*(dpl_size(my_sockets_dplist) + 2));
    assert(*pfds != NULL);
   
    //printf("\nValid PFDS on pointer %p\n\n", *pfds);
    //printf("Initial useable size of pfds (in bytes): %ld\n", malloc_usable_size(*pfds));
    //printf("Got memory reallocated on pointer %p\n\n", (*pfds));

    //printf("STEP 1:    adding pfds[0] = (fd=%d, events=%d, revents = %d)\n", server_pfd->fd, server_pfd->events, server_pfd->revents);

    //printf("pfds (size: %ld blocks) at address: %p\n", malloc_usable_size(*pfds), (*pfds));
    
    //printf("Server pollFD at address: %p\n", &(*pfds)[0]);
    //printf("Server pollFD.fd at address: %p\n", &(*pfds)[0].fd);
    //printf("Server pollFD.events at address: %p\n", &(*pfds)[0].events);
    //printf("Server pollFD.revents at address: %p\n", &(*pfds)[0].revents);
    (*pfds)[0].fd = server_pfd->fd;             //
    (*pfds)[0].events = server_pfd->events;     // deepcopy the server pfd to the first pollfd in pfds
    (*pfds)[0].revents = server_pfd->revents;   //

    
    //printf("Useable size of pfds after listen sock added (in bytes): %ld\n", malloc_usable_size(*pfds));
    my_socket_t* dummy = NULL;
    for (int i = 0; i < dpl_size(my_sockets_dplist); i++) {     // for each socket in dplist, deepcopy the file descriptors into the pfds
        dummy = dpl_get_element_at_index(my_sockets_dplist, i);
        //printf("sensor %d pollFD at address: %p\n", i+1, &((*pfds)[i+1]));
        //printf("sensor %d pollFD.fd at address: %p\n", i+1, &((*pfds)[i+1]).fd);
        //printf("sensor %d pollFD.fd value at address: %d\n", i+1, ((*pfds)[i+1]).fd);
        //printf("sensor %d pollFD.events at address: %p\n", i+1, &((*pfds)[i+1]).events);

        //printf("sensor %d pollFD.events value at address: %d\n", i+1, ((*pfds)[i+1]).events);
        //printf("sensor %d pollFD.revents at address: %p\n", i+1, &((*pfds)[i+1]).revents);
        //printf("sensor %d pollFD.revents value: %d\n", i+1, ((*pfds)[i+1]).revents);
        //printf("STEP %d:    adding pfds[%d]: ",i+2, i+2);
        //element_print(dummy); 
        if (tcp_get_sd(dummy->socket , &((*pfds)[i+1].fd)) != TCP_NO_ERROR)  exit(1);
        (*pfds)[i+1].events = SENSOR_LISTEN;
        (*pfds)[i+1].revents = 0;
        
        //printf("              added pfds[%d] = (fd=%d, events=%d, revents = %d)\n", i+1, (*pfds)[i+1].fd, (*pfds)[i+1].events , (*pfds)[i+1].revents);
        //printf("Useable size of pfds after sensor sock added at index %d (in bytes): %ld\n", i+1, malloc_usable_size(*pfds));
        
    }
    //printf("\n---------NEW PFDS:-------- \n");
    //for (int i = 0; i < dpl_size(my_sockets_dplist)+1; i++) {
        //printf("%d\n", ((*pfds)[i]).events);
      //  printf("            pfds[%d] = (fd=%d, events=%d, revents = %d)\n", i, (*pfds)[i].fd, (*pfds)[i].events, (*pfds)[i].revents);
    //}
    
    return 0;
}

/**********************************************************MY_SOCKET*********************************************************************/

void my_socket_create(my_socket_t** my_socket, tcpsock_t* socket){
    assert(*my_socket ==NULL);
    *my_socket  = (my_socket_t*) calloc(1, sizeof(my_socket_t));
    assert(*my_socket !=NULL);
    (*my_socket)->socket = socket;
    time(&((*my_socket)->lm));  
}


void my_socket_destroy(my_socket_t** my_socket, bool disconnect){
    if (*my_socket == NULL) return;
    else {
        if (disconnect) tcp_close(&(((my_socket_t*)my_socket)->socket));
        free(*my_socket);
        *my_socket = NULL;
    }
}


void my_socket_print(my_socket_t* socket) {
    printf("socket with (tcpsock_t* socket = %p\n)", socket->socket);
    fflush(stdin);
}

/** Searches for socket with sid in the given socket_dplist
 * \param socket_dplist The list in which to search for the socket
 * \param sid The ID of the socket to look for
 * \return pointer to the socket in dplist.
 * \return NULL when the socket was not found.
*/
//my_socket_t* my_socket_search(dplist_t* socket_dplist);

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
    printf("copying socket %p with sd = %d\n", ((my_socket_t*)element)->socket, sd);
    my_socket_t* copy = (my_socket_t*) calloc(1, sizeof(my_socket_t));
    printf("ELEMENT_COPY: Asked for %ld useable blocks\n", sizeof(my_socket_t)),
    assert(copy != NULL);
    printf("ELEMENT_COPY: Got %ld usable blocks for copied element.\n", malloc_usable_size(copy));
    printf("ELEMENT_COPY: Address of new_element.socket: %p\n", &(copy->socket));
    printf("ELEMENT_COPY: Address of new_element.lm: %p\n\n", &(copy->lm));
    copy->socket = ((my_socket_t*)element)->socket; //
    copy->lm = ((my_socket_t*)element)->lm; 
    //printf("ELEMENT_COPY pointer: %p\n", copy->socket);
    //printf("ORIGINAL MY_SOCKET pointer: %p\n", ((my_socket_t*)element)->socket);
    return (void *) copy;
}

void element_free(void ** element) {
    if (*element == NULL) return;
    
    else {
        int sd = 0;
        if (tcp_get_sd((*((my_socket_t**)element))->socket, &sd)) exit(0); 
        printf("freeing socket %p with sd = %d\n", (*((my_socket_t**)element))->socket, sd);
//
        //printf("ELEMENT_FREE: Address of timed_out_element.socket: %p\n", &(*((my_socket_t**)element))->socket);
        //printf("ELEMENT_FREE: Address of timed_out_element.lm: %p\n\n", &(*((my_socket_t**)element))->lm);
        
        
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





