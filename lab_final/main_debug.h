#include "callbacks/callbacks.h"

#ifndef _MAIN_H_
#define _MAIN_H_


#define GET_CALLBACK_FUNCTION_NAME(f_pointer)                                           \
        char* f_name;                                                                   \
        do {if (f_pointer == write_file) asprintf(&f_name,"'write_file'");              \
            else if (f_pointer == read_file) asprintf(&f_name,"'read_file'");           \
            else {sprintf(f_name,"unknown callback");}                                  \
        } while(0)

#endif

#define FREE_CALLBACK_FUNCTION_NAME free(f_name)  