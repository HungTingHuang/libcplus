/******************************************************************
* @file: cplus.h
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#ifndef __CPLUS_H__
#define __CPLUS_H__
#ifndef __GNUC__
#error This library only supports GCC
#endif

#include "cplus_typedef.h"
#include "cplus_atomic.h"
#include "cplus_helper.h"
#include "cplus_sys.h"
#include "cplus_systime.h"
#include "cplus_memmgr.h"
#include "cplus_mempool.h"
#include "cplus_data.h"
#include "cplus_sharedmem.h"
#include "cplus_llist.h"
#include "cplus_mutex.h"
#include "cplus_pevent.h"
#include "cplus_rwlock.h"
#include "cplus_semaphore.h"
#include "cplus_task.h"
#include "cplus_taskpool.h"
#include "cplus_syslog.h"
#include "cplus_socket.h"
#include "cplus_ipc_server.h"
#include "cplus_file.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cplus_object_option
{
    CPLUS_OBJECT_OPTION_PREALLOC,
    CPLUS_OBJECT_OPTION_THREAD_SAFE,
    CPLUS_OBJECT_OPTION_CROSS_PROCESS,
} CPLUS_OBJECT_OPTION;

int32_t cplus_object_delete(cplus_object object);

int32_t cplus_lock_shlock(cplus_object object, uint32_t timeout);
int32_t cplus_lock_exlock(cplus_object object, uint32_t timeout);
int32_t cplus_lock_unlock(cplus_object object);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_H__