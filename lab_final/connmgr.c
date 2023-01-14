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
#include <assert.h>

#include "connmgr.h"




#include "lib/tcpsock.h"
#include "lib/dplist.h"
#include <poll.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/syscall.h>

/*********************************************************CORE FUNCTIONALITY*******************************************************************/

#define INIT_PFDA 5   //Initial size of polling array
#define SERVER_LISTEN POLLIN & POLLHUP
#define SENSOR_LISTEN POLLIN
#define POLL_TIMEOUT -1 //No-event Timeout (in miliseconds). Event defined as adding a sensor or receiving sensor data.


#ifndef TIMEOUT
//#error Please define TIMEOUT!
#define TIMEOUT 5
#endif

typedef struct {
    tcpsock_t* socket;                    
    time_t lm;                              
} my_client_t;

typedef struct pollfd pollfd_t;

typedef struct my_connection {
    tcpsock_t* server; // socket for incoming clients
    dplist_t* my_sockets_dplist;    // sockets of established clients
    pollfd_t *pfd_server;           // pollfd for the incoming clients. 'static' during connection lifetime.
    pollfd_t *pfds;                 // pollfds for incoming & established clients of this connection.
    bool timeout;                   // connection timeout flag
    bool close;                     // connection close flag
    connmgr_rx_data_t* rx_data;
} my_connection_t;



void connmgr_listen(int port_number) {
    //open receiving file.
    int status = 0;
    FILE *sensor_data_recv = fopen("sensor_data_recv", "w+"); // w+ deletes the content of the file and creates it if it doesn't exist.
    setbuf(sensor_data_recv, NULL); // Unbuffered, data appears in file as soon as it is written
    my_connection_t* connection;
    connmgr_rx_data_t* data = NULL;
    myconn_init(&connection, port_number);
    status = myconn_listen(connection);
    while(status != CONNMGR_CLOSE) {

        myconn_get_rx_data(connection, &data);
        //printf("got here\n");
        for (int i = 0; i<data->buff_size; i++) {
            if((data->pollrx_buffer_flag)[i] == true) {
            printf("sensor id = %" PRIu16 " - temperature = %lf - timestamp = %ld\n", (&((data->pollrx_buffer)[i]))->id, 
                                                                                     (&((data->pollrx_buffer)[i]))->value, 
                                                                                     (&((data->pollrx_buffer)[i]))->ts);
            PRINTF_CONNMGR("size of rx_buffer: %d", connection->rx_data->buff_size); 
                                            
            }
            fprintf(sensor_data_recv,"%"PRIu16 " %lf %ld\n",(&((data->pollrx_buffer)[i]))->id,
                                                     (&((data->pollrx_buffer)[i]))->value, 
                                                     (&((data->pollrx_buffer)[i]))->ts);
        }
        status = myconn_listen(connection);
    }
    // close remaining connections
    myconn_destroy(&connection);
    fclose(sensor_data_recv);
}

void connmgr_free() {
    return;
}

/*********************************************************CALLBACKS*******************************************************************/
void * my_client_copy(void * element) {
    int sd = 0;
     if (tcp_get_sd(((my_client_t*)element)->socket, &sd) != TCP_NO_ERROR) exit(0);
    my_client_t* copy = (my_client_t*) calloc(1, sizeof(my_client_t));
    assert(copy != NULL);
    copy->socket = ((my_client_t*)element)->socket; //
    copy->lm = ((my_client_t*)element)->lm; 
    return (void *) copy;
}

void my_client_free(void ** element) {
    if (*element == NULL) return;
    
    else {
        int sd = 0;
        if (tcp_get_sd((*((my_client_t**)element))->socket, &sd)) exit(0); 
        printf("freeing socket %p with sd = %d\n", (*((my_client_t**)element))->socket, sd);
        tcp_close(&((*((my_client_t**)element))->socket));
        free((*(my_client_t**)element)->socket);
        free(*element);
        *element = NULL;
    }
}

int my_client_compare(void * x, void * y) {
    return ((((my_client_t*)x)->lm < ((my_client_t*)y)->lm) ? -1 : // sort by sid
            ((((my_client_t*)x)->lm == ((my_client_t*)y)->lm) && ((((my_client_t*)x)->socket == ((my_client_t*)y)->socket))) ? 0 : //only equal when socket == socket & lm == lm
             1);
}

void my_client_print(void* element) {
    char buff[25];
    struct tm* timeinfo;
    timeinfo = localtime(&(((my_client_t*)element)->lm));
    strftime(buff, 25, "%c", timeinfo);
    int sd;
    tcp_get_sd((tcpsock_t*)((my_client_t*)element)->socket , &sd);
    printf(" (socket_addr=%p, socket_fd=%d, lm = %s)\n",  
     (tcpsock_t*)((my_client_t*)element)->socket, sd, buff);
}

void my_socket_print(my_client_t* socket) {
    printf("socket with (tcpsock_t* socket = %p\n)", socket->socket);
    //fflush(stdin); -> undefined behaviour of fflush(stdin) on linux systems.
}

/**********************************************************_MY CONNECTION FUNCTIONALITY_*********************************************************************/

int myconn_pfds_update(my_connection_t* connection) {
    pollfd_t * new_pfds =  (pollfd_t*) realloc(connection->pfds, (sizeof(pollfd_t)*(dpl_size(connection->my_sockets_dplist)) + 2));
    ERROR_IF(new_pfds == NULL, ERR_MALLOC('realloc pollfd_t'));
    //if (new_pfds == connection->pfds) return 1; -> cppcheck: condition is always false
    //else {
        connection->pfds = new_pfds;
        (connection->pfds)[0].fd = connection->pfd_server->fd;             //
        (connection->pfds)[0].events = connection->pfd_server->events;     // deepcopy the server pfd to the first pollfd in pfds
        (connection->pfds)[0].revents = connection->pfd_server->revents;   //

        my_client_t* dummy = NULL;
        for (int i = 0; i < dpl_size(connection->my_sockets_dplist); i++) {     // for each socket in dplist, deepcopy the file descriptors into the pfds
            dummy = dpl_get_element_at_index(connection->my_sockets_dplist, i);
            if (tcp_get_sd(dummy->socket , &((connection->pfds)[i+1].fd)) != TCP_NO_ERROR)  exit(1);
            (connection->pfds)[i+1].events = SENSOR_LISTEN;
            (connection->pfds)[i+1].revents = 0;
        }
        return 0;
    //}
}

int myconn_rxbuff_update(my_connection_t* connection) {
    int count = 0;
    sensor_data_t * new_pollrx_buffer =  (sensor_data_t *) realloc(connection->rx_data->pollrx_buffer, (sizeof(sensor_data_t)*(dpl_size(connection->my_sockets_dplist)) + 2));
    ERROR_IF(new_pollrx_buffer == NULL, ERR_MALLOC('realloc pollrx_buffer'));
    //if (new_pollrx_buffer == connection->rx_data->pollrx_buffer) return 1; -> cppcheck: condition is always false
    
    bool* new_pollrx_buffer_flags =  (bool*) realloc(connection->rx_data->pollrx_buffer_flag, (sizeof(bool)*(dpl_size(connection->my_sockets_dplist)) + 2));
    ERROR_IF(new_pollrx_buffer_flags == NULL, ERR_MALLOC('realloc pollrx_buffer_flags'));
    //if (new_pollrx_buffer_flags == connection->rx_data->pollrx_buffer_flag) return 1; -> cppcheck: condition is always false

    connection->rx_data->pollrx_buffer_flag = new_pollrx_buffer_flags; 
    connection->rx_data->pollrx_buffer = new_pollrx_buffer; 

    for (int i = 0; i < dpl_size(connection->my_sockets_dplist); i++) {
        (connection->rx_data->pollrx_buffer)[i].id = 0;
        (connection->rx_data->pollrx_buffer)[i].value = 0;
        (connection->rx_data->pollrx_buffer)[i].ts = 0;
        (connection->rx_data->pollrx_buffer_flag)[i] = false;
        count++;
    }
    connection->rx_data->buff_size = count;
    return 0;
    
    
}

int myconn_init(my_connection_t** connection, int port_number) {
    *connection = (my_connection_t*) calloc(1, sizeof(my_connection_t));
    ERROR_IF(*connection == NULL, ERR_MALLOC("my_connection_t"));
    ERROR_IF((tcp_passive_open(&((*connection)->server), port_number) != TCP_NO_ERROR), "init connection error");  
    (*connection)->timeout = false;
    (*connection)->close = false;
    (*connection)->my_sockets_dplist = dpl_create(my_client_copy, my_client_free, my_client_compare, my_client_print);

    (*connection)->pfd_server = (pollfd_t*) calloc(1, sizeof(pollfd_t));
    ERROR_IF((*connection)->pfd_server == NULL, ERR_MALLOC("pollfd_t"));
    ERROR_IF((tcp_get_sd((*connection)->server, &((*connection)->pfd_server->fd)) != TCP_NO_ERROR), "socket not bound!\n");         
    (*connection)->pfd_server->events = POLLIN;  
    
    (*connection)->pfds = (pollfd_t *) calloc(1, sizeof(pollfd_t));
    ERROR_IF((*connection)->pfds == NULL, ERR_MALLOC("pfds"));
    
    (*connection)->rx_data = (connmgr_rx_data_t*) calloc(1, sizeof(connmgr_rx_data_t));
    ERROR_IF((*connection)->rx_data == NULL, ERR_MALLOC("connmgr_rx_data_t"));
    (*connection)->rx_data->pollrx_buffer = (sensor_data_t*) calloc(1, sizeof(sensor_data_t));
    ERROR_IF((*connection)->rx_data->pollrx_buffer == NULL, ERR_MALLOC("pollrx_buffer_flag"));
    (*connection)->rx_data->pollrx_buffer_flag = (bool *) calloc(1, sizeof(bool));
    ERROR_IF((*connection)->rx_data->pollrx_buffer_flag == NULL, ERR_MALLOC("pollrx_buffer_flag"));
    (*connection)->rx_data->buff_size = 1;
    
    myconn_pfds_update(*connection); 
    myconn_rxbuff_update(*connection);
    
    return 0;
}

int myconn_destroy(my_connection_t** connection) {
    free((*connection)->rx_data->pollrx_buffer_flag);
    free((*connection)->rx_data->pollrx_buffer);
    free((*connection)->rx_data);
    free((*connection)->pfds);
    dpl_free(&((*connection)->my_sockets_dplist), true);
    free((*connection)->pfd_server);
    tcp_close(&((*connection)->server));
    free(*connection);
    (*connection) = NULL;
    return 0;
}

int myconn_add_client(my_connection_t* connection, my_client_t* client) {
    dpl_insert_at_index(connection->my_sockets_dplist, client, -1, true); 
    myconn_pfds_update(connection);
    myconn_rxbuff_update(connection);
    return 0;
}

int myconn_remove_client(my_connection_t* connection, my_client_t* client) {
    dpl_remove_element(connection->my_sockets_dplist, client, true); 
    myconn_pfds_update(connection);
    myconn_rxbuff_update(connection);
    return 0;
}

int myconn_get_rx_data(my_connection_t* connection, connmgr_rx_data_t** rx_data_ptr) {
    ERROR_IF(connection == NULL, "param 'connection' is NULL!");
    if (rx_data_ptr == NULL) {ERROR_IF(rx_data_ptr == NULL, "param 'rx_data' is NULL!"); exit(1);}
    if ((*rx_data_ptr == NULL))(*rx_data_ptr) = (connection->rx_data);
    else ERROR_IF(1, "param 'rx_data' is not NULL!");
    return 0;
}

int myconn_listen(my_connection_t* connection) {
    my_client_t* client = NULL;
    int bytes;
    int tcp_result;
    tcpsock_t* new_client_socket = NULL;
    my_client_t new_client = {.socket = NULL, .lm = 0};
    //PRINTF_CONNMGR("Address of connection->timeout: %p", &(connection->timeout));
    if (connection->timeout) connection->close = true; // if timeout was not reset in previous loop, mark the while loop to close.
    connection->timeout = true;

    for (int i = 0; i < dpl_size(connection->my_sockets_dplist); i++) {
            (connection->rx_data->pollrx_buffer_flag)[i] = false; // reset pollrx flags
    }

    while ((!(connection->timeout)) || (!(connection->close))) {
        ERROR_IF(poll(connection->pfds, dpl_size(connection->my_sockets_dplist) + 1, TIMEOUT*1000) == -1, "Polling failed\n");        // BLOCK until something happens, or timeout! -> puts current thread/process to sleep. TIMEOUT expressed in seconds but attr is in miliseconds.            
        // after POLL_TIMEOUT, check for events on server pfd
        //PRINTF_CONNMGR("Address of connection->pfds: %p", &(connection->pfds));
        if(((connection->pfds)[0]).revents & POLLIN) {                                                                               
            if (tcp_wait_for_connection(connection->server, &new_client_socket) != TCP_NO_ERROR) exit(EXIT_FAILURE);     
            new_client.socket = new_client_socket;
            time(&(new_client.lm));
            myconn_add_client(connection, &new_client);
            connection->timeout = false;    
            connection->close = false;                      
        } 

        // after POLL_TIMEOUT, loop connected clients
        
        for (int i = 1; i<(dpl_size(connection->my_sockets_dplist) + 1); i++) {       
            client = dpl_get_element_at_index(connection->my_sockets_dplist, i-1);

            // check for events on current client
            //PRINTF_CONNMGR("Address of connection->pfds: %p", &(connection->pfds));
            if(((connection->pfds)[i]).revents & POLLIN) {
                //printf("Received sensor data!\n");
                 // read sensor ID

                // insert data into my connection 
                bytes = sizeof(((connection->rx_data->pollrx_buffer)[i-1]).id);
                tcp_result = tcp_receive(client->socket, (void *) &(((connection->rx_data->pollrx_buffer)[i-1]).id), &bytes);
                ERROR_IF(tcp_result == TCP_SOCKET_ERROR, "CONNMGR: Socket is NULL or not yet bound");
                ERROR_IF(tcp_result == TCP_CONNECTION_CLOSED, "CONNMGR: Connection closed");
                ERROR_IF(tcp_result == TCP_SOCKOP_ERROR, "CONNMGR: Socket error occured while receiving data");
                // read temperature
                bytes = sizeof(((connection->rx_data->pollrx_buffer)[i-1]).value);
                tcp_result = tcp_receive(client->socket, (void *) &(((connection->rx_data->pollrx_buffer)[i-1]).value), &bytes);
                ERROR_IF(tcp_result == TCP_SOCKET_ERROR, "CONNMGR: Socket is NULL or not yet bound");
                ERROR_IF(tcp_result == TCP_CONNECTION_CLOSED, "CONNMGR: Connection closed");
                ERROR_IF(tcp_result == TCP_SOCKOP_ERROR, "CONNMGR: Socket error occured while receiving data");
                // read timestamp
                bytes = sizeof(((connection->rx_data->pollrx_buffer)[i-1]).ts);
                tcp_result = tcp_receive(client->socket, (void *) &(((connection->rx_data->pollrx_buffer)[i-1]).ts), &bytes);
                ERROR_IF(tcp_result == TCP_SOCKET_ERROR, "CONNMGR: Socket is NULL or not yet bound");
                ERROR_IF(tcp_result == TCP_CONNECTION_CLOSED, "CONNMGR: Connection closed");
                ERROR_IF(tcp_result == TCP_SOCKOP_ERROR, "CONNMGR: Socket error occured while receiving data");
                if ((tcp_result == TCP_NO_ERROR) && bytes) {
                    (connection->rx_data->pollrx_buffer_flag)[i-1] = true;
                    PRINTF_CONNMGR("%s: sensor id = %" PRIu16 " - temperature = %lf - timestamp = %ld\n","Connmgr:", ((connection->rx_data->pollrx_buffer)[i-1]).id, ((connection->rx_data->pollrx_buffer)[i-1]).value,((long int)((connection->rx_data->pollrx_buffer)[i-1]).ts)); // set flag that data has been received onthis position in the buffer.
                    
                    //PRINTF_CONNMGR(DMSG_DATA("CONNMGR",((connection->rx_data->pollrx_buffer)[i-1])));
                    
                    //printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", 
                                        //data.id, 
                                        //data.value, 
                                        //(long int) data.ts);
                    time(&(client->lm));    
                    connection->timeout = false;
                    connection->close = false;
                }
            }
            //PRINTF_CONNMGR("Address of client->lm: %p", &(client->lm));
            if ((time(NULL)-(client->lm)) >= TIMEOUT) { // time_t exprssed in seconds
               myconn_remove_client(connection, client);
            }
        }
        return CONNMGR_SUCCESS;
    }
    return CONNMGR_CLOSE;
    
}




