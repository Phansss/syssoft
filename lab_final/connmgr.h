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


/** Starts listening on the given port and when when a sensor node connects it writes the data to a sensor_data_recv file.
 * \param port_number port to listen to
*/
void connmgr_listen(int port_number);

/** clean up the connmgr and free all used memory
 * 
*/
void connmgr_free();

#endif

