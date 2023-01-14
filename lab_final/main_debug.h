#ifndef _MAIN_H_
#define _MAIN_H_

int write_file(cb_args_t* data_arg);
int read_file(cb_args_t* data_arg);
int datamgr_process(cb_args_t* data_arg);
int dbinsert(cb_args_t* data_arg);
int connmgr_poll(cb_args_t* data_arg);

#define DEBUG_GET_CALLBACK_FUNCTION_NAME(f_pointer)                                           \
        char* f_name = NULL;                                                                   \
        do {if (f_pointer == write_file) asprintf(&f_name,"'write_file'");              \
            else if (f_pointer == read_file) asprintf(&f_name,"'read_file'");           \
            else if (f_pointer == dbinsert) asprintf(&f_name,"'db_insert_sensor'");           \
            else if (f_pointer == connmgr_poll) asprintf(&f_name,"'connmgr_poll'");           \
            else if (f_pointer == datamgr_process) asprintf(&f_name,"'datamgr_process'");           \
            else {sprintf(f_name,"unknown callback");}                                  \
        } while(0)


#define DEBUG_FREE_CALLBACK_FUNCTION_NAME free(f_name)  



#endif

