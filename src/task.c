/******************************************************************
* @file: task.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#include <pthread.h>
#include <limits.h>
#include <sys/resource.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_pevent.h"
#include "cplus_systime.h"
#include "cplus_atomic.h"
#include "cplus_task.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 2)
#define MAX_WAIT_STOP_TIME (60 * 1000)
#define SUPPORT_NON_PORTABLE 0


struct task
{
    uint16_t type;
    pthread_t thread;
    CPLUS_TASK_PROC proc;
    CPLUS_TASK_PROC callkback;
    void * param1;
    void * param2;
    uint32_t duration;
    volatile bool is_suspended;
    cplus_pevent evt_start;
    cplus_pevent evt_stop;
    cplus_pevent evt_finish;
    cplus_pevent evt_pause;
};

static pthread_once_t once_init = PTHREAD_ONCE_INIT;

static pthread_key_t key_for_evt_finish;
static pthread_key_t key_for_duration;
static pthread_key_t key_for_last_timestamp;

static void cplus_task_once_init(void)
{
    pthread_key_create(&key_for_evt_finish, NULL);
    pthread_key_create(&key_for_duration, NULL);
    pthread_key_create(&key_for_last_timestamp, NULL);
}

uint32_t cplus_task_get_loop_last_timestamp(void)
{
    int32_t * ptr = NULL;
    if ((ptr = (int32_t *)pthread_getspecific(key_for_last_timestamp)))
    {
        return cplus_atomic_read(ptr);
    }
    return 0;
}

int32_t cplus_task_set_loop_finish(void)
{
    uint32_t res = CPLUS_SUCCESS;
    cplus_task evet_finish = NULL;

    if ((evet_finish = (cplus_task)pthread_getspecific(key_for_evt_finish)))
    {
        res = cplus_pevent_set(evet_finish);
    }

    return res;
}

int32_t cplus_task_reset_loop_finish(void)
{
    uint32_t res = CPLUS_SUCCESS;
    cplus_task evet_finish = NULL;

    if ((evet_finish = (cplus_task)pthread_getspecific(key_for_evt_finish)))
    {
        res = cplus_pevent_reset(evet_finish);
    }

    return res;
}

int32_t cplus_task_set_loop_duration(uint32_t duration)
{
    uint32_t * ptr = NULL;

    if ((ptr = (uint32_t *)pthread_getspecific(key_for_duration)))
    {
        cplus_atomic_write(ptr, duration);
    }

    return CPLUS_SUCCESS;
}

uint32_t cplus_task_get_loop_duration(void)
{
    uint32_t * ptr = NULL;

    if ((ptr = (uint32_t *)pthread_getspecific(key_for_duration)))
    {
        return cplus_atomic_read(ptr);
    }
    return 0;
}

uintptr_t cplus_task_get_pid(cplus_task obj)
{
    CHECK_OBJECT_TYPE(obj);
    return ((struct task *)(obj))->thread;
}

int32_t cplus_task_set_duration(cplus_task obj, uint32_t duration)
{
    CHECK_OBJECT_TYPE(obj);
    cplus_atomic_write(&(((struct task *)(obj))->duration), duration);
    return CPLUS_SUCCESS;
}

uint32_t cplus_task_get_duration(cplus_task obj)
{
    CHECK_OBJECT_TYPE(obj);
    return cplus_atomic_read(&(((struct task *)(obj))->duration));
}

int32_t cplus_task_wait_start(cplus_task obj, uint32_t timeout)
{
    CHECK_OBJECT_TYPE(obj);
    return cplus_pevent_wait(((struct task *)(obj))->evt_start, timeout);
}

int32_t cplus_task_wait_finish(cplus_task obj, uint32_t timeout)
{
    CHECK_OBJECT_TYPE(obj);
    return cplus_pevent_wait(((struct task *)(obj))->evt_finish, timeout);
}

int32_t cplus_task_start(cplus_task obj, uint32_t delay)
{
    CHECK_OBJECT_TYPE(obj);

    cplus_systime_sleep_msec(delay);
    return cplus_pevent_set(((struct task *)(obj))->evt_start);
}

int32_t cplus_task_delete(cplus_task obj)
{
    struct task * task = (struct task *)(obj);
    CHECK_OBJECT_TYPE(obj);

    if (task->evt_start)
    {
        cplus_pevent_delete(task->evt_start);
    }
    if (task->evt_stop)
    {
        cplus_pevent_delete(task->evt_stop);
    }
    if (task->evt_finish)
    {
        cplus_pevent_delete(task->evt_finish);
    }
    if (task->evt_pause)
    {
        cplus_pevent_delete(task->evt_pause);
    }
    cplus_free(task);

    return CPLUS_SUCCESS;
}

int32_t cplus_task_stop(cplus_task obj, uint32_t timeout)
{
    int32_t res = CPLUS_FAIL;
    struct task * task = (struct task *)(obj);
    uint32_t tout = (CPLUS_INFINITE_TIMEOUT == timeout)? MAX_WAIT_STOP_TIME: timeout;
    CHECK_OBJECT_TYPE(obj);

    if (0 != task->thread
        AND CPLUS_INFINITE_TIMEOUT != task->duration)
    {
        cplus_pevent_set(task->evt_stop);
        if (false == cplus_pevent_get_status(task->evt_start))
        {
            cplus_pevent_set(task->evt_start);
        }
#if SUPPORT_NON_PORTABLE
        if (0 == timeout)
        {
            res = pthread_tryjoin_np(task->thread, NULL);
        }
        else
        {
            struct timespec ts = {0};
            cplus_systime_get_abstime_after_msec(&ts, tout);
            res = pthread_timedjoin_np(task->thread, NULL, &ts);
        }

        if (0 != res)
        {
            errno = res;
            if (0 != (res = pthread_cancel(task->thread)))
            {
                errno = res;
            }
            res = CPLUS_FAIL;
        }
#else
        res = cplus_pevent_wait(task->evt_finish, tout);
        if (CPLUS_FAIL == res)
        {
            pthread_cancel(task->thread);
            errno = (0 == tout)? EBUSY: ETIMEDOUT;
        }
        else
        {
            pthread_join(task->thread, NULL);
        }
#endif // SUPPORT_NON_PORTABLE
        cplus_task_delete(task);
        return res;
    }

    return CPLUS_SUCCESS;
}

int32_t cplus_task_pause(cplus_task obj, bool pause)
{
    CHECK_OBJECT_TYPE(obj);

    if (pause ^ !(cplus_pevent_get_status(((struct task *)(obj))->evt_pause)))
    {
        return (pause)? cplus_pevent_reset(((struct task *)(obj))->evt_pause)
            : cplus_pevent_set(((struct task *)(obj))->evt_pause);
    }
    return CPLUS_SUCCESS;
}

void cancel_in_routine(void * param)
{
    struct task * task = (struct task *)(param);

    if (CPLUS_INFINITE_TIMEOUT == task->duration)
    {
        if (task->callkback)
        {
            task->callkback(task->param1, task->param2);
        }

        cplus_task_delete(task);
    }
    return;
}

void * task_executor(void * param)
{
    struct task * task = (struct task *)(param);
    uint32_t last_timestamp = 0, duration = 0;

    pthread_once(&(once_init), cplus_task_once_init);
    (void)pthread_setspecific(key_for_evt_finish, task->evt_finish);
    (void)pthread_setspecific(key_for_duration, &task->duration);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push(cancel_in_routine, task);

    int32_t start_working = cplus_pevent_wait(
        task->evt_start
        , CPLUS_INFINITE_TIMEOUT);

    /*  Reason for False Positive : There is no double_unlock because "task->evt_stop->ptr_mutex"
        must be locked, and if it is not locked, will not call to unlock */
    /* coverity[double_unlock: FALSE] */
    while((0 == start_working)
        AND ((CPLUS_FAIL == cplus_pevent_wait(task->evt_stop, 0)) AND (EAGAIN == errno))
        AND (cplus_pevent_get_status(task->evt_pause)
            OR (CPLUS_SUCCESS == cplus_pevent_wait(task->evt_pause, CPLUS_INFINITE_TIMEOUT))))
    {
        pthread_testcancel();

        last_timestamp = cplus_systime_get_tick();
        (void)pthread_setspecific(key_for_last_timestamp, &(last_timestamp));

        cplus_pevent_reset(task->evt_finish);
        task->proc(task->param1, task->param2);
        cplus_pevent_set(task->evt_finish);

        if (CPLUS_INFINITE_TIMEOUT == (duration = cplus_atomic_read(&task->duration)))
        {
            break;
        }

        uint32_t diff_timestamp = cplus_systime_elapsed_tick(last_timestamp);
        if (duration > diff_timestamp)
        {
            cplus_pevent_wait(task->evt_stop, duration - diff_timestamp);
        }
        else
        {
            cplus_systime_sleep_msec(1);
        }
    }
    cplus_pevent_set(task->evt_finish);

    pthread_exit(0);
    pthread_cleanup_pop(0);
}

static void * task_initialize_object(CPLUS_TASK_CONFIG config)
{
    int32_t res = 0;
    pthread_attr_t * attr = NULL;
    struct task * task = NULL;

    if ((task = (struct task *)cplus_malloc(sizeof(struct task))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(task);
        task->type = OBJ_TYPE;
        task->thread = 0;
        task->proc = config->proc;
        task->callkback = config->callback;
        task->param1 = config->param1;
        task->param2 = config->param2;
        task->duration = config->duration;
        task->is_suspended = config->suspend;

        if (NULL == (task->evt_start = cplus_pevent_new(
            true, !task->is_suspended)))
        {
            goto exit;
        }
        if (NULL == (task->evt_stop = cplus_pevent_new(true, false)))
        {
            goto exit;
        }
        if (NULL == (task->evt_finish = cplus_pevent_new(false, false)))
        {
            goto exit;
        }
        if (NULL == (task->evt_pause = cplus_pevent_new(true, true)))
        {
            goto exit;
        }

        pthread_attr_t attr_stacksize = {0};
        if (0 < config->stacksize)
        {
            if (0 == pthread_attr_init(&attr_stacksize))
            {
                if (0 == pthread_attr_setstacksize(&attr_stacksize, config->stacksize))
                {
                    attr = &attr_stacksize;
                }
            }
        }
        res = pthread_create(&task->thread, attr, task_executor, task);

        if (attr)
        {
            pthread_attr_destroy(attr);
        }

        if (0 != res)
        {
            errno = res;
            goto exit;
        }

        if (CPLUS_INFINITE_TIMEOUT == config->duration)
        {
            pthread_detach(task->thread);
        }
    }
    return task;
exit:
    cplus_task_delete(task);
    return NULL;
}

bool cplus_task_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

cplus_task cplus_task_oneshot(
    CPLUS_TASK_PROC proc
    , void * param1
    , void * param2)
{
    CPLUS_TASK_CONFIG_T config = {0};
    CHECK_NOT_NULL(proc, NULL);

    config.proc = proc;
    config.callback = NULL;
    config.param1 = param1;
    config.param2 = param2;
    config.duration = CPLUS_INFINITE_TIMEOUT;
    config.suspend = false;
    config.stacksize = 0;
    return task_initialize_object(&config);
}

cplus_task cplus_task_oneshot_ex(
    CPLUS_TASK_PROC proc
    , void * param1
    , void * param2
    , CPLUS_TASK_PROC callback)
{
    CPLUS_TASK_CONFIG_T config = {0};
    CHECK_NOT_NULL(proc, NULL);

    config.proc = proc;
    config.callback = callback;
    config.param1 = param1;
    config.param2 = param2;
    config.duration = CPLUS_INFINITE_TIMEOUT;
    config.suspend = false;
    config.stacksize = 0;
    return task_initialize_object(&config);
}

cplus_task cplus_task_new(
    CPLUS_TASK_PROC proc
    , void * param1
    , void * param2
    , uint32_t duration)
{
    CPLUS_TASK_CONFIG_T config = {0};
    CHECK_NOT_NULL(proc, NULL);

    config.proc = proc;
    config.callback = NULL;
    config.param1 = param1;
    config.param2 = param2;
    config.duration = duration;
    config.suspend = true;
    config.stacksize = 0;
    return task_initialize_object(&config);
}

cplus_task cplus_task_new_ex(CPLUS_TASK_CONFIG config)
{
    CHECK_NOT_NULL(config, NULL);
    CHECK_NOT_NULL(config->proc, NULL);

    if (0 != config->stacksize)
    {
        struct rlimit rlim;
        getrlimit(RLIMIT_STACK, &rlim);

        CHECK_IN_INTERVAL(config->stacksize, PTHREAD_STACK_MIN, rlim.rlim_cur, NULL);
    }

    return task_initialize_object(config);
}

uintptr_t cplus_task_get_self_id(void)
{
	return pthread_self();
}

#ifdef __CPLUS_UNITTEST__

void test_back(void * param1, void * param2)
{
    int32_t * time = (int32_t *)param1;
    bool * failed = (bool *)param2;

    UNITTEST_EXPECT_EQ(1, 1 == *time);
}

void test_proc(void * param1, void * param2)
{
    int32_t * time = (int32_t *)param1;

    for (int32_t i = 0; i < *time; i++)
    {
        cplus_systime_sleep_msec(1000);
    }
}

CPLUS_UNIT_TEST(cplus_task_oneshot_ex, functionity)
{
    cplus_task task = NULL;
    int32_t repect = 1;

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_oneshot_ex(test_proc, &repect, failed, test_back)));
    cplus_systime_sleep_msec(1500);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_task_new, functionity)
{
    cplus_task task = NULL;
    uint32_t time, total_spent;
    int32_t repect = 5; // run 5 second

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_proc, &repect, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_start(task, 0));
    UNITTEST_EXPECT_EQ(EAGAIN, errno);
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_start(task, 500));
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    time = cplus_systime_get_tick();
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 500));
    time = cplus_systime_elapsed_tick(time);
    total_spent = cplus_systime_get_tick();
    UNITTEST_EXPECT_EQ(true, time >= 500 AND time < 550);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    total_spent = cplus_systime_elapsed_tick(total_spent);
    UNITTEST_EXPECT_EQ(true, total_spent <= 5500);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_task_set_duration, functionity)
{
    cplus_task task = NULL;
    int32_t repect = 5; // run 5 second

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_proc, &repect, failed, 100)));
    UNITTEST_EXPECT_EQ(100, cplus_task_get_duration(task));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 500));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_set_duration(task, 500));
    UNITTEST_EXPECT_EQ(500, cplus_task_get_duration(task));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void test_set_loop_duration_proc(void * param1, void * param2)
{
    bool * failed = (bool *)param2;

    UNITTEST_EXPECT_EQ(100, cplus_task_get_loop_duration());
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_set_loop_duration(500));
    UNITTEST_EXPECT_EQ(500, cplus_task_get_loop_duration());
}

CPLUS_UNIT_TEST(cplus_task_set_loop_duration, functionity)
{
    cplus_task task = NULL;

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_set_loop_duration_proc, NULL, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_finish(task, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(500, cplus_task_get_duration(task));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void test_set_loop_finish_proc(void * param1, void * param2)
{
    bool * failed = (bool *)param2;

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_set_loop_finish());
    while(1) {};
}

CPLUS_UNIT_TEST(cplus_task_set_loop_finish, functionity)
{
    cplus_task task = NULL;
    uint32_t time;

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_set_loop_finish_proc, NULL, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_finish(task, 0));
    UNITTEST_EXPECT_EQ(EAGAIN, errno);
    time = cplus_systime_get_tick();
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_finish(task, 1500));
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_finish(task, CPLUS_INFINITE_TIMEOUT));

    time = cplus_systime_get_tick();
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_stop(task, 2000));
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(true, time >= 2000 AND time < 2500);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void test_get_loop_last_timestamp_proc(void * param1, void * param2)
{
    bool * failed = (bool *)param2;

    UNITTEST_EXPECT_NE(0, cplus_task_get_loop_last_timestamp());
}

CPLUS_UNIT_TEST(cplus_task_get_loop_last_timestamp, functionity)
{
    cplus_task task = NULL;

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_get_loop_last_timestamp_proc, NULL, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
    cplus_systime_sleep_msec(1000);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void test_task_stop_and_wait_proc(void * param1, void * param2)
{
    int32_t * time = (int32_t *)param1;

    for (int32_t i = 0; i < *time; i++)
    {
        cplus_systime_sleep_msec(1000);
    }
}

CPLUS_UNIT_TEST(cplus_task_stop, functionity)
{
    cplus_task task = NULL;
    int32_t repect = 3;
    uint32_t time;

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_task_stop_and_wait_proc, &repect, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
    time = cplus_systime_get_tick();
    cplus_systime_sleep_msec(1000);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(true, time >= 3000 && time < 3050);

    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_task_stop_and_wait_proc, &repect, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
    time = cplus_systime_get_tick();
    cplus_systime_sleep_msec(1000);
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_stop(task, 0));
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(EBUSY, errno);
    UNITTEST_EXPECT_EQ(true, time >= 1000 && time < 1050);

    repect = 90;
    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_task_stop_and_wait_proc, &repect, failed, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
    time = cplus_systime_get_tick();
    cplus_systime_sleep_msec(1000);
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(true, time >= 60000 && time < 61050);
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void test_pause_proc(void * param1, void * param2)
{
    int32_t * count = (int32_t *)param1;
    UNUSED_PARAM(param2);

    (* count) += 1;
}

CPLUS_UNIT_TEST(cplus_task_pause, functionity)
{
    cplus_task task = NULL;
    int32_t count = 0;
    UNITTEST_EXPECT_EQ(true, NULL != (task = cplus_task_new(test_pause_proc, &count, NULL, 100)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
    while (1)
    {
        if (5 == count)
        {
            UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_pause(task, true));
            break;
        }
    }
    cplus_systime_sleep_msec(2000);
    UNITTEST_EXPECT_EQ(5, count);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_pause(task, false));
    while (1)
    {
        if (10 == count)
        {
            UNITTEST_EXPECT_EQ(10, count);
            UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_pause(task, true));
            break;
        }
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_task(void)
{
    UNITTEST_ADD_TESTCASE(cplus_task_oneshot_ex, functionity);
    UNITTEST_ADD_TESTCASE(cplus_task_new, functionity);
    UNITTEST_ADD_TESTCASE(cplus_task_set_duration, functionity);
    UNITTEST_ADD_TESTCASE(cplus_task_set_loop_duration, functionity);
    UNITTEST_ADD_TESTCASE(cplus_task_set_loop_finish, functionity);
    // UNITTEST_ADD_TESTCASE(cplus_task_get_loop_last_timestamp, functionity);
    UNITTEST_ADD_TESTCASE(cplus_task_stop, functionity);
    UNITTEST_ADD_TESTCASE(cplus_task_pause, functionity);
}

#endif // __CPLUS_UNITTEST__
