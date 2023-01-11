#include "callbacks.h"
#include "../connmgr.h"


#define _connmgr_attr_ ((connmgr_attr_t*)data_arg)

typedef struct {
    tcpsock_t* socket;                        // 
    time_t lm;                               // timestamp last modified
} my_socket_t;

typedef struct pollfd pollfd_t;

typedef struct connmgr_attr {
    data_buffer_t* sbuff_data_buffer;
    my_connection_t* connection;
} connmgr_attr_t;


int connmgr_poll(cb_args_t* data_arg) {    
        connmgr_rx_data_t** rx_data_array;
        if(myconn_listen(((connmgr_attr_t*)data_arg)->connection) == CONNMGR_CLOSE) return DATA_PROCESS_ERROR;
        myconn_get_rx_data(((connmgr_attr_t*)data_arg)->connection, rx_data_array);
        if((*rx_data_array)->buff_size > ((connmgr_attr_t*)data_arg)->sbuff_data_buffer->size) {
            //TO DO: write function in sbuffer.h to resize callback function buffer. 
        }
        for (int i; i<(*data)->buff_size; i++) {
            if(((*data)->pollrx_buffer_flag)[i] == true) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", (((*data)->pollrx_buffer)[i]).id, 
                                                                                     (((*data)->pollrx_buffer)[i]).value, 
                                                                                     (((*data)->pollrx_buffer)[i]).ts);                            
            }
        }


        ((connmgr_attr_t*)data_arg)->connection->
        
    }

}

int connmgr_init(my_connection_t** myconnection, int port_number) {
    myconn_init(myconnection, port_number);

}

int connmgr_cleanup(connmgr_attr_t** attr){
    return;
}