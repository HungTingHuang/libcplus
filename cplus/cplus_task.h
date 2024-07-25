#ifndef __CPLUS_TASK_H__
#define __CPLUS_TASK_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (* CPLUS_TASK_PROC) (void * param1, void * param2);

typedef struct cplus_task_config
{
    CPLUS_TASK_PROC proc;
    CPLUS_TASK_PROC callback;
    void * param1;
    void * param2;
    uint32_t duration;
    bool suspend;
    uint32_t stacksize;
} *CPLUS_TASK_CONFIG, CPLUS_TASK_CONFIG_T;

cplus_task cplus_task_oneshot(
    CPLUS_TASK_PROC proc
    , void * param1
    , void * param2);

cplus_task cplus_task_oneshot_ex(
    CPLUS_TASK_PROC proc
    , void * param1
    , void * param2
    , CPLUS_TASK_PROC callback);

cplus_task cplus_task_new(
    CPLUS_TASK_PROC proc
    , void * param1
    , void * param2
    , uint32_t duration);

cplus_task cplus_task_new_ex(CPLUS_TASK_CONFIG config);

int32_t cplus_task_delete(cplus_task obj);

bool cplus_task_check(cplus_object obj);
uintptr_t cplus_task_get_pid(cplus_task obj);
uintptr_t cplus_task_get_self_id(void);

int32_t cplus_task_start(cplus_task obj, uint32_t delay);
int32_t cplus_task_stop(cplus_task obj, uint32_t timeout);
int32_t cplus_task_pause(cplus_task obj, bool pause);

int32_t cplus_task_set_duration(cplus_task obj, uint32_t duration);
uint32_t cplus_task_get_duration(cplus_task obj);

int32_t cplus_task_wait_start(cplus_task obj, uint32_t timeout);
int32_t cplus_task_wait_finish(cplus_task obj, uint32_t timeout);

int32_t cplus_task_set_loop_duration(uint32_t duration);
uint32_t cplus_task_get_loop_duration(void);
int32_t cplus_task_set_loop_finish(void);
int32_t cplus_task_reset_loop_finish(void);
uint32_t cplus_task_get_loop_last_timestamp(void);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_TASK_H__