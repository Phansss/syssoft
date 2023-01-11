#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include "../sbuffer.h" //lab 8
#include "../connmgr.h" //lab7
#include "../sensor_db.h" //lab6
#include "../datamgr.h" //lab 5

/*___________________Sbuffer Writers -> read from resources_______________ */
typedef struct connmgr_attr connmgr_attr_t;
int connmgr(cb_args_t* data_arg);
int connmgr_init(connmgr_attr_t**attr);
int connmgr_cleanup(connmgr_attr_t** attr);

typedef struct read_file read_file_t;
int read_file(cb_args_t* data_arg);
int read_file_init(read_file_t** attr);
int read_file_cleanup(read_file_t** attr);


/*___________________Sbuffer Readers -> write to resources_______________ */
typedef struct write_file write_file_t; 
int write_file(cb_args_t* data_arg);
int write_file_init(write_file_t** attr);
int write_file_cleanup(write_file_t** attr);

typedef struct dbinsert dbinsert_t;
int dbinsert(cb_args_t*data_arg);
int dbinsert_init(dbinsert_t** attr, char clear_up_flag);
int dbinsert_cleanup(dbinsert_t** attr);

typedef struct datamgr_process datamgr_process_t;
int datamgr_process(cb_args_t* data_arg);
int datamgr_init(datamgr_process_t** attr);
int datamgr_cleanup(datamgr_process_t** attr);


#endif