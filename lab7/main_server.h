/**
 * \author Pieter Hanssens
 */
#ifndef __MAIN_SERVER_H__
#define __MAIN_SERVER_H__
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "connmgr.h"
#include "errmacros.h"
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


/** Forks the shell and . Parent process returns to the shell, child process calls connmgr_listen. 
 * 
*/
int fork_and_listen(int port);

int stop_connmgr(pid_t** child_pid);

void final_message(void);
int add_sensor_node(char* sid, char* st, char* server_ip, char* port);

#endif