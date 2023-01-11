
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_
#include "config.h"
#ifdef DEBUG_DPLIST
#define PRINTF_DPLIST(...) _DEBUG_PRINTF_(__VA_ARGS__, " ");
#else
#define PRINTF_DPLIST(...) (void)0
#endif

#ifdef DEBUG_SENSOR_DB
#define PRINTF_SENSOR_DB(...) _DEBUG_PRINTF_(__VA_ARGS__, " ");
#else
#define PRINTF_SENSOR_DB(...) (void)0
#endif

#ifdef DEBUG_DATAMGR
#define PRINTF_DATAMGR(...) _DEBUG_PRINTF_(__VA_ARGS__, " ");
#else
#define PRINTF_DATAMGR(...) (void)0
#endif

#ifdef DEBUG_CONNMGR
#define PRINTF_CONNMGR(...) _DEBUG_PRINTF_(__VA_ARGS__, " ");
#else
#define PRINTF_CONNMGR(...) (void)0
#endif

#ifdef DEBUG_SBUFFER
#define PRINTF_SBUFFER(...) _DEBUG_PRINTF_(__VA_ARGS__, " ");
#else
#define PRINTF_SBUFFER(...)  (void)0
#endif

#ifdef DEBUG_MAIN
#define PRINTF_MAIN(...) _DEBUG_PRINTF_(__VA_ARGS__, " ");
#else
#define PRINTF_MAIN(...) (void)0
#endif
               
#define _DEBUG_PRINTF_(fmt,...) 									                                                                 \
        do {pid_t* tid = (pid_t*) malloc(sizeof(pid_t));                                                                             \
                if (tid == NULL) {                                                                                                   \
                    fprintf(stderr, "DEBUGGER: [%s:%d] in %s: "fmt"%s\n",  __FILE__,__LINE__,__func__, __VA_ARGS__);                 \
                    fprintf(stderr,"   NOTE: Could not determine thread id due to malloc error");                                    \
                    fflush(stderr);                                                                                                  \
                    break;                                                                                                           \
                }                                                                                                                    \
                *tid = syscall(__NR_gettid);                                                                                         \
                fprintf(stderr, "DEBUGGER: [%s:%d] in %s_(%d): "fmt"%s\n",  __FILE__,__LINE__,__func__ ,*tid, __VA_ARGS__);          \
                fflush(stderr);                                                                                                      \
                free(tid);                                                                                                           \
            }while(0)            


#define DMSG_DATA(source_file, data) printf("%s: sensor id = %" PRIu16 " - temperature = %lf - timestamp = %ld\n",     \
                                        source_file,                                                                   \
                                        (data).id,                                                                     \
                                        (data).value,                                                                  \
                                        ((long int)(data)).ts);                                                        
#define DMSG_MUTEX_LOCKTRY(thr,lock) #thr " tries lock: " #lock
#define DMSG_MUTEX_LOCKACQ(thr,lock) #thr " acquires lock: " #lock
#define DMSG_MUTEX_UNLOCK(thr,lock) #thr " unlocks: " #lock
#define DMSG_BARRIER_BLOCK(thr,lock) #thr " blocks on: " #lock
#define DMSG_BARRIER_UNBLOCK(thr) #thr " continues. "

#define DMSG_SBUFFER_INSERT_DATA(data) printf("Inserting data in sbuffer: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", (data).id, (data).value, ((long int)(data)).ts)

#endif  //_DEBUGGER_H_