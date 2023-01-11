/**
 * \author Pieter Hanssens
 */
#ifndef __CONNMGR_H__
#define __CONNMGR_H__

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "lib/dplist.h"
#include <poll.h>
#include <inttypes.h>
#include "lib/tcpsock.h"

#define CONNMGR_CLOSE 1
#define CONNMGR_SUCCESS 0

#define DMSG_DATA(source_file, data) printf("%s: sensor id = %" PRIu16 " - temperature = %lf - timestamp = %ld\n","hi", (data).id, (data).value, ((long int)(data)).ts); 

typedef struct my_connection my_connection_t;
typedef struct {
    sensor_data_t* pollrx_buffer;                        // 
    bool* pollrx_buffer_flag;                               // timestamp last modified
    int buff_size;
} connmgr_rx_data_t;

void connmgr_listen(int port_number);
void connmgr_free();

int myconn_init(my_connection_t** connection, int port_number);
int myconn_destroy(my_connection_t** connection);
int myconn_listen(my_connection_t* connection) ;
int myconn_get_rx_data(my_connection_t* connection, connmgr_rx_data_t** rx_data_ptr);


#endif

