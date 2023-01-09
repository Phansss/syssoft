#include "callbacks.h"
#include <poll.h>
#include <inttypes.h>
#include "lib/tcpsock.h"

#define _connmgr_attr_ ((connmgr_attr_t*)data_arg)

typedef struct {
    tcpsock_t* socket;                        // 
    time_t lm;                               // timestamp last modified
} my_socket_t;

typedef struct pollfd pollfd_t;

typedef struct connmgr_attr {
    sensor_data_t* data;
    // Variables for incoming new connection:
    tcpsock_t* passive_open_socket;
    tcpsock_t* new_socket_dummy;
    my_socket_t* my_socket_element;
    //Variables to receive data from a client sensor:
    my_socket_t* client;
    int bytes;
    int result;
    // Variables to control the while loop: Stop while loop if (timeout == true and close == true):
    bool timeout;   
    bool close;
    pollfd_t *pfds;
    dplist_t* my_sockets_dplist;
    pollfd_t server_pfd;
    dplist_t* my_sockets_dplist;
} connmgr_attr_t;

int pfds_recreate(pollfd_t** pfds, pollfd_t* server_pfd, dplist_t* my_sockets_dplist);
void* element_copy(void * element);                                             
void element_free(void ** element);   
int element_compare(void * x, void * y);
void element_print(void * element);  
void my_socket_print(my_socket_t* socket); 

int connmgr(cb_args_t* data_arg) {    

    while (!((_connmgr_attr_->timeout)) || (!(_connmgr_attr_->close))) {
        //WHILE CONTROL
        if ((_connmgr_attr_->timeout)) _connmgr_attr_->close = true; // if timeout was not reset in previous loop, mark the while loop to close.
        (_connmgr_attr_->timeout) = true;

        POLL_ERROR(poll(_connmgr_attr_->pfds, dpl_size(_connmgr_attr_->my_sockets_dplist) + 1, CONNMGR_TIMEOUT*1000));        // BLOCK until something happens, or timeout! -> puts current thread/process to sleep. TIMEOUT expressed in seconds but attr is in miliseconds.            
        
        // after POLL_TIMEOUT, check for events on server pfd
        if((_connmgr_attr_->pfds)[0].revents & POLLIN) {                                                                               
            if (tcp_wait_for_connection(_connmgr_attr_->passive_open_socket, &(_connmgr_attr_->new_socket_dummy)) != TCP_NO_ERROR) exit(EXIT_FAILURE);            // new_socket_dummy is on heap.
            
            //create my_socket_element and insert in my_sockets_list.
            _connmgr_attr_->my_socket_element->socket = _connmgr_attr_->new_socket_dummy;
            time(&(_connmgr_attr_->my_socket_element->lm));
            dpl_insert_at_index(_connmgr_attr_->my_sockets_dplist, _connmgr_attr_->my_socket_element, -1, true);

            //recreate pfds 
            pfds_recreate(&(_connmgr_attr_->pfds), &(_connmgr_attr_->server_pfd), _connmgr_attr_->my_sockets_dplist);

            //WHILE CONTROL
            _connmgr_attr_->timeout = false;    
            _connmgr_attr_->close = false;                      
        } 

        // after POLL_TIMEOUT, loop connected clients
        for (int i = 1; i<(dpl_size(_connmgr_attr_->my_sockets_dplist) + 1); i++) {       
            (_connmgr_attr_->client) = dpl_get_element_at_index(_connmgr_attr_->my_sockets_dplist, i-1);

            // check for events on current client
            if((_connmgr_attr_->pfds)[i].revents & POLLIN) {
                //printf("Received sensor data!\n");
                 // read sensor ID
                _connmgr_attr_->bytes = sizeof(_connmgr_attr_->data->id);
                _connmgr_attr_->result = tcp_receive(_connmgr_attr_->client->socket, (void *) &(_connmgr_attr_->data->id), &(_connmgr_attr_->bytes));
                // read temperature
                _connmgr_attr_->bytes = sizeof(_connmgr_attr_->data->value);
                _connmgr_attr_->result = tcp_receive(_connmgr_attr_->client->socket, (void *) &(_connmgr_attr_->data->value), &(_connmgr_attr_->bytes));
                // read timestamp
                _connmgr_attr_->bytes = sizeof(_connmgr_attr_->data->ts);
                _connmgr_attr_->result = tcp_receive(_connmgr_attr_->client->socket, (void *) &(_connmgr_attr_->data->ts), &(_connmgr_attr_->bytes));
                
                if (((_connmgr_attr_->result) == TCP_NO_ERROR) && (_connmgr_attr_->bytes)) {
                    printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", (_connmgr_attr_->data->id), _connmgr_attr_->data->value, (long int) _connmgr_attr_->data->ts);
                    //fprintf(sensor_data_recv,"%hd %lf %ld\n",data.id,data.value, data.ts); 
                    time(&((_connmgr_attr_->client)->lm)); //update socket last modified time
                    //WHILE CONTROL
                    (_connmgr_attr_->timeout) = false; // something happened on one of the sensors -> reset timeout and close.
                    (_connmgr_attr_->close)= false;
                    return DATA_PROCESS_SUCCESS;
                }
            }
            if ((time(NULL)-(_connmgr_attr_->client->lm)) >= CONNMGR_TIMEOUT) { // time_t exprssed in seconds
                dpl_remove_element(_connmgr_attr_->my_sockets_dplist, _connmgr_attr_->client, true); // Remove and free element using element_free(). -> tcp connection closed.
                dpl_print(_connmgr_attr_->my_sockets_dplist);
                //recreate pfds 
                pfds_recreate(&(_connmgr_attr_->pfds), &(_connmgr_attr_->server_pfd), _connmgr_attr_->my_sockets_dplist);
            }
        }
    } 
}

int connmgr_init(connmgr_attr_t** attr) {
    (*attr)->data = NULL;
    // Variables for incoming new connection:
    (*attr)->passive_open_socket = NULL;
    if (tcp_passive_open(&((*attr)->passive_open_socket), CONNMGR_PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);          
    (*attr)->new_socket_dummy = NULL;
    (*attr)->my_socket_element->socket = NULL;
    (*attr)->my_socket_element->lm = 0;
    //Variables to receive data from a client sensor:
    (*attr)->client = NULL;
    (*attr)->bytes = 0;
    (*attr)->result = 0;
    // Variables to control the while loop: Stop while loop if (timeout == true and close == true):
    (*attr)->timeout = false;   
    (*attr)->close = false;
    ((*attr)->server_pfd).fd = 0;
    ((*attr)->server_pfd).events = POLLIN;
    ((*attr)->server_pfd).revents = 0;
    if (tcp_get_sd((*attr)->passive_open_socket, &(((*attr)->server_pfd).fd)) != TCP_NO_ERROR) printf("socket not bound!\n");         

    //create my_sockets_dplist
    (*attr)->my_sockets_dplist = dpl_create(element_copy, element_free, element_compare, element_print);
    //create pfds from server_pfd + my_sockets dplist
    (*attr)->pfds = (pollfd_t *) calloc((dpl_size((*attr)->my_sockets_dplist) + 1), sizeof(pollfd_t));
    pfds_recreate(&((*attr)->pfds), &((*attr)->server_pfd), (*attr)->my_socket_element);
}

int connmgr_cleanup(connmgr_attr_t** attr){
    return;
}

/********************************************************** _HELPER FUNCTION_ *********************************************************************/
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
        (*pfds)[i+1].events = POLLIN;
        (*pfds)[i+1].revents = 0;
    }
    return 0;
}


/********************************************************* _DPLIST_CALLBACKS_ *******************************************************************/
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
