

#ifndef _MAIN_H_
#define _MAIN_H_


#define GET_CALLBACK_FUNCTION_NAME(f_pointer)                                                      \
        char* f_name;                                                                   \
        do {if (f_pointer == rd_write_to_file) asprintf(&f_name,"'rd_write_to_file'");         \
            else if (f_pointer == wr_read_from_file) asprintf(&f_name,"'wr_read_from_file'");    \
            else {sprintf(f_name,"unknown callback");}                                       \
        } while(0)

#endif

#define FREE_CALLBACK_FUNCTION_NAME free(f_name)  