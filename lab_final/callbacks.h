#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_


#include "sbuffer.h" //lab 8
#include "connmgr.h" //lab7
#include "sensor_db.h" //lab6
#include "datamgr.h" //lab 5
#include "lib/dplist.h" //lab4
#include "lib/tcpsock.h" //lab 3

/*Sbuffer Writers*/
typedef struct connmgr_attr connmgr_attr_t;
int connmgr(cb_args_t* data_arg);
int connmgr_init(connmgr_attr_t**attr);
int connmgr_cleanup(connmgr_attr_t** attr);

typedef struct read_file read_file_t;
int read_file(cb_args_t* data_arg);
int read_file_init(read_file_t** attr);
int read_file_cleanup(read_file_t** attr);


/*Sbuffer Writers*/
typedef struct write_file write_file_t; 
int write_file(cb_args_t* data_arg);
int write_file_init(write_file_t** attr);
int write_file_cleanup(write_file_t** attr);



#endif