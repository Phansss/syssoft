#ifndef _ERRHANDLER_H_
#define _ERRHANDLER_H_
#include <sys/types.h> 
#include <sys/syscall.h>
#include <unistd.h>
#include "config.h"

#define ERR_SEMAPHORE_INIT "Error during semaphore initialization"
#define ERR_BARRIER_INIT "Error during barrier initialization" 
#define ERR_RWLOCK_INIT "Error during rwlock initialization"
#define ERR_MUTEX_INIT "Error during mutex initialization"
#define ERR_MALLOC(type) "Not enough heap memory available for " #type 


#define ERROR_IF(...) _ERROR_IF_(__VA_ARGS__, " ");   
#define _ERROR_IF_(cond,fmt,...) 									                                                                 \
        do {if (cond) {                                                                                                              \
                pid_t* tid = (pid_t*) malloc(sizeof(pid_t));                                                                         \
                if (tid != NULL) {                                                                                                   \
                    *tid = syscall(__NR_gettid);                                                                                     \
                    fprintf(stderr, "ERROR [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,*tid, __VA_ARGS__);    \
                    fprintf(stderr,"    "#cond"\n");                                                                                 \
                    fflush(stderr);                                                                                                  \
                    free(tid);                                                                                                       \
                    ERROR_EXIT;                                                                                                      \
                    }                                                                                                                \
                else {                                                                                                               \
                    fprintf(stderr, "ERROR_SBUFF [%s:%d] in %s: "fmt"%s\n",  __FILE__,__LINE__,__func__, __VA_ARGS__);               \
                    fprintf(stderr,"    "#cond"\n");                                                                                 \
                    fprintf(stderr,"   NOTE: Could not determine thread id due to malloc error");                                    \
                    fflush(stderr);                                                                                                  \
                    ERROR_EXIT;                                                                                                      \
                }                                                                                                                    \
            }                                                                                                                        \
        } while(0)


#endif