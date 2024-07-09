#ifndef __CPLUS_TYPEDEF_H__
#define __CPLUS_TYPEDEF_H__
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CPLUS_FAIL -1
#define CPLUS_SUCCESS 0

#ifndef UINT32_MAX
    #define UINT32_MAX (0U - 1)
#endif

#ifndef RSIZE_MAX_MEM
    #define RSIZE_MAX_MEM (256UL << 20) /* 256MB */
#endif

#define CPLUS_INFINITE_TIMEOUT (0U - 1)

typedef void* cplus_object;
typedef void* cplus_data;
typedef void* cplus_ipc_server;
typedef void* cplus_ipc_client;
typedef void* cplus_llist;
typedef void* cplus_mempool;
typedef void* cplus_mutex;
typedef void* cplus_pevent;
typedef void* cplus_rwlock;
typedef void* cplus_semaphore;
typedef void* cplus_sharedmem;
typedef void* cplus_socket;
typedef void* cplus_syslog;
typedef void* cplus_task;
typedef void* cplus_taskpool;
typedef void* cplus_file;

#endif //__CPLUS_TYPEDEF__