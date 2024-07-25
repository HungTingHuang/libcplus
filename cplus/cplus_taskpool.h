#ifndef __CPLUS_TASKPOOL_H__
#define __CPLUS_TASKPOOL_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cplus_taskpool_config
{
    uint32_t worker_count;
    uint32_t max_task_count;
    uint32_t stack_size;
    bool get_task_cycling;
} *CPLUS_TASKPOOL_CONFIG, CPLUS_TASKPOOL_CONFIG_T;

typedef struct cplus_taskpool_task
{
    CPLUS_TASK_PROC proc;
    void * param1;
    void * param2;
    CPLUS_TASK_PROC callback;
} *CPLUS_TASKPOOL_TASK, CPLUS_TASKPOOL_TASK_T;

cplus_taskpool cplus_taskpool_new(uint32_t worker_count);
cplus_taskpool cplus_taskpool_new_ex(CPLUS_TASKPOOL_CONFIG config);

int32_t cplus_taskpool_delete(cplus_taskpool obj);
int32_t cplus_taskpool_delete_ex(cplus_taskpool obj, uint32_t timeout);

bool cplus_taskpool_check(cplus_object obj);

int32_t cplus_taskpool_add_task_ex(cplus_taskpool obj, CPLUS_TASKPOOL_TASK task);
int32_t cplus_taskpool_add_task(cplus_taskpool obj, CPLUS_TASK_PROC proc, void * param1);

int32_t cplus_taskpool_remove_task(
    cplus_taskpool obj
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg);

uint32_t cplus_taskpool_get_worker_count(cplus_taskpool obj);
int32_t cplus_taskpool_reset_worker_count(cplus_taskpool obj, uint32_t worker_count);

uint32_t cplus_taskpool_get_task_count(cplus_taskpool obj);

int32_t cplus_taskpool_all_pause(cplus_taskpool obj, bool pause);

int32_t cplus_taskpool_clear_task(cplus_taskpool obj);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_TASKPOOL_H__