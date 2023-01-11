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
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#define DEBUG_CONNMGR

#ifdef DEBUG_CONNMGR                                                  
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
            }                                                                                                                        \
            }while(0)
#define DEBUG_PRINTF(...) _DEBUG_NO_ARG(__VA_ARGS__, " ");
#define _DEBUG_NO_ARG(fmt,...) (void)0
#endif

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
            DEBUG_PRINTF("size of rx_buffer: %d", connection->rx_data->buff_size) 
                                            
            }
            fprintf(sensor_data_recv,"%hd %lf %ld\n",(&((data->pollrx_buffer)[i]))->id,
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
void * element_copy(void * element) {
    int sd = 0;
     if (tcp_get_sd(((my_client_t*)element)->socket, &sd) != TCP_NO_ERROR) exit(0);
    my_client_t* copy = (my_client_t*) calloc(1, sizeof(my_client_t));
    assert(copy != NULL);
    copy->socket = ((my_client_t*)element)->socket; //
    copy->lm = ((my_client_t*)element)->lm; 
    return (void *) copy;
}

void element_free(void ** element) {
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

int element_compare(void * x, void * y) {
    return ((((my_client_t*)x)->lm < ((my_client_t*)y)->lm) ? -1 : // sort by sid
            ((((my_client_t*)x)->lm == ((my_client_t*)y)->lm) && ((((my_client_t*)x)->socket == ((my_client_t*)y)->socket))) ? 0 : //only equal when socket == socket & lm == lm
             1);
}

void element_print(void* element) {
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
    fflush(stdin);
}

/**********************************************************_MY CONNECTION FUNCTIONALITY_*********************************************************************/

int myconn_pfds_update(my_connection_t* connection) {
    pollfd_t * new_pfds =  (pollfd_t*) realloc(connection->pfds, sizeof(pollfd_t)*(dpl_size(connection->my_sockets_dplist) + 2));
    ERROR_IF(new_pfds == NULL, ERR_MALLOC('realloc pollfd_t'));
    if (new_pfds == NULL) return 1;
    else {
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
    }
}

int myconn_rxbuff_update(my_connection_t* connection) {
    int count = 0;
    sensor_data_t * new_pollrx_buffer =  (sensor_data_t *) realloc(connection->rx_data->pollrx_buffer, sizeof(sensor_data_t)*(dpl_size(connection->my_sockets_dplist) + 2));
    ERROR_IF(new_pollrx_buffer == NULL, ERR_MALLOC('realloc pollrx_buffer'));
    if (new_pollrx_buffer == NULL) return 1;
    
    bool* new_pollrx_buffer_flags =  (bool*) realloc(connection->rx_data->pollrx_buffer_flag, sizeof(bool)*(dpl_size(connection->my_sockets_dplist) + 2));
    ERROR_IF(new_pollrx_buffer_flags == NULL, ERR_MALLOC('realloc pollrx_buffer_flags'));
    if (new_pollrx_buffer_flags == NULL) return 1;

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
    (*connection)->my_sockets_dplist = dpl_create(element_copy, element_free, element_compare, element_print);

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
    ERROR_IF(rx_data_ptr == NULL, "param 'rx_data' is NULL!");
    (*rx_data_ptr) = (connection->rx_data);
    return 0;
}

int myconn_listen(my_connection_t* connection) {
    my_client_t* client = NULL;
    int bytes = 0;
    int result = 0;
    tcpsock_t* new_client_socket = NULL;
    my_client_t new_client = {.socket = NULL, .lm = 0};
    //DEBUG_PRINTF("Address of connection->timeout: %p", &(connection->timeout))
    if (connection->timeout) connection->close = true; // if timeout was not reset in previous loop, mark the while loop to close.
    connection->timeout = true;

    for (int i = 0; i < dpl_size(connection->my_sockets_dplist); i++) {
            (connection->rx_data->pollrx_buffer_flag)[i] = false; // reset pollrx flags
    }

    while ((!(connection->timeout)) || (!(connection->close))) {
        POLL_ERROR(poll(connection->pfds, dpl_size(connection->my_sockets_dplist) + 1, TIMEOUT*1000));        // BLOCK until something happens, or timeout! -> puts current thread/process to sleep. TIMEOUT expressed in seconds but attr is in miliseconds.            

        // after POLL_TIMEOUT, check for events on server pfd
        //DEBUG_PRINTF("Address of connection->pfds: %p", &(connection->pfds))
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
            //DEBUG_PRINTF("Address of connection->pfds: %p", &(connection->pfds))
            if(((connection->pfds)[i]).revents & POLLIN) {
                //printf("Received sensor data!\n");
                 // read sensor ID

                // insert data into my connection 
                bytes = sizeof(((connection->rx_data->pollrx_buffer)[i-1]).id);
                result = tcp_receive(client->socket, (void *) &(((connection->rx_data->pollrx_buffer)[i-1]).id), &bytes);
                // read temperature
                bytes = sizeof(((connection->rx_data->pollrx_buffer)[i-1]).value);
                result = tcp_receive(client->socket, (void *) &(((connection->rx_data->pollrx_buffer)[i-1]).value), &bytes);
                // read timestamp
                bytes = sizeof(((connection->rx_data->pollrx_buffer)[i-1]).ts);
                result = tcp_receive(client->socket, (void *) &(((connection->rx_data->pollrx_buffer)[i-1]).ts), &bytes);

                if ((result == TCP_NO_ERROR) && bytes) {
                    (connection->rx_data->pollrx_buffer_flag)[i-1] = true;
                    printf("%s: sensor id = %" PRIu16 " - temperature = %lf - timestamp = %ld\n","Connmgr:", ((connection->rx_data->pollrx_buffer)[i-1]).id, ((connection->rx_data->pollrx_buffer)[i-1]).value,((long int)((connection->rx_data->pollrx_buffer)[i-1]).ts)); // set flag that data has been received onthis position in the buffer.
                    
                    //DEBUG_PRINTF(DMSG_DATA("CONNMGR",((connection->rx_data->pollrx_buffer)[i-1])));
                    
                    //printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", 
                                        //data.id, 
                                        //data.value, 
                                        //(long int) data.ts);
                    time(&(client->lm));    
                    connection->timeout = false;
                    connection->close = false;
                }
            }
            //DEBUG_PRINTF("Address of client->lm: %p", &(client->lm))
            if ((time(NULL)-(client->lm)) >= TIMEOUT) { // time_t exprssed in seconds
               myconn_remove_client(connection, client);
            }
        }
        printf("returning\n");
        return CONNMGR_SUCCESS;
    }
    return CONNMGR_CLOSE;
    
}




