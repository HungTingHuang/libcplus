/******************************************************************
* @file: taskpool.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <limits.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_mempool.h"
#include "cplus_llist.h"
#include "cplus_task.h"
#include "cplus_taskpool.h"
#include "cplus_mutex.h"
#include "cplus_semaphore.h"
#include "cplus_systime.h"
#include "cplus_pevent.h"
#include "cplus_atomic.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 3)
#define MAX_TASK_COUNT 255U
#define MAX_WORKER_COUNT 32U

#define TIMEOUT_FOR_WAIT_RECEIVED_TASK 3000
#define TIMEOUT_FOR_TERMINAL_WORKER (1000 * 15)
#define PERIOD_FOR_WORKER_FREQUENCY 1

struct taskpool
{
    uint16_t type;
    uint32_t stack_size;
    bool get_task_cycling;
    cplus_llist task_list;
    cplus_mempool task_pool;
    cplus_mutex task_access_sect;
    cplus_mutex worker_access_sect;
    cplus_llist worker_list;
    cplus_mempool worker_pool;
    cplus_semaphore remain_taskpool;
};

struct task_worker
{
    pthread_t pid;
    cplus_task executor;
};

int32_t cplus_taskpool_delete_ex(cplus_taskpool obj, uint32_t timeout)
{
    struct taskpool * tp = (struct taskpool *)obj;

    CHECK_OBJECT_TYPE(obj);
    if (tp->worker_list)
    {
        struct task_worker * worker = NULL;
        cplus_crit_sect_enter(tp->worker_access_sect);
        while (NULL != (worker = cplus_llist_pop_back(tp->worker_list)))
        {
            cplus_task_stop(worker->executor, timeout);
            cplus_mempool_free(tp->worker_pool, worker);
        }
        cplus_llist_delete(tp->worker_list);
        cplus_crit_sect_exit(tp->worker_access_sect);
    }

    if (tp->worker_pool)
    {
        cplus_mempool_delete(tp->worker_pool);
    }

    if (tp->worker_access_sect)
    {
        cplus_mutex_delete(tp->worker_access_sect);
    }

    if (tp->task_list)
    {
        CPLUS_TASKPOOL_TASK * task = NULL;
        cplus_crit_sect_enter(tp->task_access_sect);
        while (NULL != (task = cplus_llist_pop_back(tp->task_list)))
        {
            cplus_mempool_free(tp->task_pool, task);
        }
        cplus_llist_delete(tp->task_list);
        cplus_crit_sect_exit(tp->task_access_sect);
    }

    if (tp->task_pool)
    {
        cplus_mempool_delete(tp->task_pool);
    }

    if (tp->task_access_sect)
    {
        cplus_mutex_delete(tp->task_access_sect);
    }

    if (tp->remain_taskpool)
    {
        cplus_semaphore_delete(tp->remain_taskpool);
    }

    cplus_free(tp);
    return CPLUS_SUCCESS;
}

int32_t cplus_taskpool_delete(cplus_taskpool obj)
{
    return cplus_taskpool_delete_ex(obj, TIMEOUT_FOR_TERMINAL_WORKER);
}

void task_worker(void * param1, void * param2)
{
    struct taskpool * tp = (struct taskpool *)param1;
    struct cplus_taskpool_task task_t = {0}, * task = NULL;
    uint32_t task_count = 0;
    void * (* fetch_task)(cplus_llist) = NULL;
    UNUSED_PARAM(param2);

    fetch_task = (!!(tp->get_task_cycling))? cplus_llist_get_cycling_next: cplus_llist_pop_back;

    if ((true == tp->get_task_cycling)
        || CPLUS_SUCCESS == cplus_semaphore_wait_poll(tp->remain_taskpool, TIMEOUT_FOR_WAIT_RECEIVED_TASK))
    {
        cplus_crit_sect_enter(tp->task_access_sect);
        if (0 < (task_count = cplus_llist_get_size(tp->task_list)))
        {
            if (!(task = fetch_task(tp->task_list)))
            {
                cplus_systime_sleep_msec(1);
                cplus_crit_sect_exit(tp->task_access_sect);
                return;
            }
            else
            {
                cplus_mem_cpy(&task_t, task, sizeof(CPLUS_TASKPOOL_TASK_T));
                if (!(tp->get_task_cycling))
                {
                    cplus_mempool_free(tp->task_pool, task);
                }
            }
        }
        cplus_crit_sect_exit(tp->task_access_sect);

        if (task_t.proc)
        {
            task_t.proc(task_t.param1, task_t.param2);
        }

        if (task_t.callback)
        {
            task_t.callback(task_t.param1, task_t.param2);
        }
    }
    return;
}

static void * taskpool_initialize_object(
    CPLUS_TASKPOOL_CONFIG config)
{
    struct taskpool * tp = NULL;
    CPLUS_TASK_CONFIG_T task_config = {0};

    if ((tp = (struct taskpool *)cplus_malloc(sizeof(struct taskpool))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(tp);

        tp->type = OBJ_TYPE;
        tp->stack_size = config->stack_size;
        tp->get_task_cycling = config->get_task_cycling;

        tp->task_access_sect = cplus_mutex_new();
        if (NULL == tp->task_access_sect)
        {
            goto exit;
        }

        tp->task_pool = cplus_mempool_new(
            config->max_task_count
            , sizeof(CPLUS_TASKPOOL_TASK_T));
        if (NULL == tp->task_pool)
        {
            goto exit;
        }

        tp->task_list = cplus_llist_prev_new(config->max_task_count);
        if (NULL == tp->task_list)
        {
            goto exit;
        }

        tp->worker_access_sect = cplus_mutex_new();
        if (NULL == tp->worker_access_sect)
        {
            goto exit;
        }

        tp->worker_list = cplus_llist_prev_new(MAX_WORKER_COUNT);
        if (NULL == tp->worker_list)
        {
            goto exit;
        }

        tp->worker_pool = cplus_mempool_new(MAX_WORKER_COUNT, sizeof(struct task_worker));
        if (NULL == tp->worker_pool)
        {
            goto exit;
        }

        tp->remain_taskpool = cplus_semaphore_new(0);
        if (NULL == tp->remain_taskpool)
        {
            goto exit;
        }

        for (uint32_t i = 0; i < config->worker_count; i++)
        {
            cplus_task executor = NULL;
            struct task_worker * worker = NULL;

            task_config.proc = task_worker;
            task_config.param1 = tp;
            task_config.duration = PERIOD_FOR_WORKER_FREQUENCY;
            task_config.suspend = true;
            task_config.stacksize = tp->stack_size;
            if (!(executor = cplus_task_new_ex(&task_config)))
            {
                goto exit;
            }

            if ((worker = (struct task_worker *)cplus_mempool_alloc(tp->worker_pool)))
            {
                worker->pid = cplus_task_get_pid(executor);
                worker->executor = executor;
            }

            cplus_task_start(executor, 0);
            cplus_task_wait_start(executor, CPLUS_INFINITE_TIMEOUT);

            cplus_llist_push_front(tp->worker_list, worker);
        }
    }
    return tp;
exit:
    cplus_taskpool_delete(tp);
    return NULL;
}

cplus_taskpool cplus_taskpool_new(uint32_t worker_count)
{
    CPLUS_TASKPOOL_CONFIG_T config = {0};
    CHECK_IF(worker_count > MAX_WORKER_COUNT, NULL);

    config.worker_count = worker_count;
    config.max_task_count = MAX_TASK_COUNT;
    return taskpool_initialize_object(&config);
}

cplus_taskpool cplus_taskpool_new_ex(CPLUS_TASKPOOL_CONFIG config)
{
    CHECK_NOT_NULL(config, NULL);
    CHECK_IN_INTERVAL(config->max_task_count, 1, MAX_TASK_COUNT, NULL);
    CHECK_IF(config->worker_count > MAX_WORKER_COUNT, NULL);

    config->stack_size = (0 != config->stack_size)? CPLUS_MAX(((uint32_t)PTHREAD_STACK_MIN), config->stack_size): 0;
    return taskpool_initialize_object(config);
}

bool cplus_taskpool_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

int32_t cplus_taskpool_add_task_ex(cplus_taskpool obj, CPLUS_TASKPOOL_TASK task)
{
    int32_t res = CPLUS_FAIL;
    struct taskpool * tp = (struct taskpool *)obj;
    CPLUS_TASKPOOL_TASK t = NULL;

    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(task, CPLUS_FAIL);

    cplus_crit_sect_enter(tp->task_access_sect);
    uint32_t task_count = cplus_llist_get_size(tp->task_list);
    if (MAX_TASK_COUNT > task_count)
    {
        if ((t = (CPLUS_TASKPOOL_TASK)cplus_mempool_alloc(tp->task_pool)))
        {
            t->proc = task->proc;
            t->param1 = task->param1;
            t->param2 = task->param2;
            t->callback = task->callback;
            res = cplus_llist_push_front(tp->task_list, t);
        }
    }
    else
    {
        errno = ENOMEM;
        res = CPLUS_FAIL;
    }
    cplus_crit_sect_exit(tp->task_access_sect);

    if ((false == tp->get_task_cycling) && CPLUS_SUCCESS == res)
    {
        cplus_semaphore_push(tp->remain_taskpool, 1);
    }

    return res;
}

int32_t cplus_taskpool_add_task(
    cplus_taskpool obj,
    CPLUS_TASK_PROC proc,
    void * param1)
{
    CPLUS_TASKPOOL_TASK_T task = {0};
    task.proc = proc;
    task.param1 = param1;
    task.param2 = NULL;
    task.callback = NULL;

    return cplus_taskpool_add_task_ex(obj, &task);
}

int32_t cplus_taskpool_remove_task(
    cplus_taskpool obj
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    int32_t res = CPLUS_FAIL;
    struct taskpool * tp = (struct taskpool *)obj;
    uint32_t task_count = 0;
    CPLUS_TASKPOOL_TASK task = NULL;
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(comparator, CPLUS_FAIL);

    if (0 < (task_count = cplus_llist_get_size(tp->task_list)))
    {
        cplus_crit_sect_enter(tp->task_access_sect);
        {
            if ((task = cplus_llist_pop_if(tp->task_list, comparator, arg)))
            {
                cplus_mempool_free(tp->task_pool, task);
            }
        }
        cplus_crit_sect_exit(tp->task_access_sect);
    }
    res = CPLUS_SUCCESS;

    return res;
}

uint32_t cplus_taskpool_get_worker_count(cplus_taskpool obj)
{
    uint32_t worker_count = 0;
    struct taskpool * tp = (struct taskpool *)obj;
    CHECK_OBJECT_TYPE(obj);

    cplus_crit_sect_enter(tp->worker_access_sect);
    {
        worker_count = cplus_llist_get_size(tp->worker_list);
    }
    cplus_crit_sect_exit(tp->worker_access_sect);

    return worker_count;
}

int32_t cplus_taskpool_reset_worker_count(cplus_taskpool obj, uint32_t worker_count)
{
    struct taskpool * tp = (struct taskpool *)obj;
    struct task_worker * worker = NULL;
    int32_t count_to_change = 0;
    uint32_t current_worker_count = 0;
    uint32_t old_stack_size = tp->stack_size;

    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(worker_count > MAX_WORKER_COUNT, CPLUS_FAIL);

    cplus_crit_sect_enter(tp->worker_access_sect);
    {
        current_worker_count = cplus_llist_get_size(tp->worker_list);
        if (current_worker_count > worker_count)
        {
            count_to_change = current_worker_count - worker_count;
            while (count_to_change --)
            {
                if ((worker = (struct task_worker *)cplus_llist_pop_back(tp->worker_list)))
                {
                    cplus_task_stop(worker->executor, TIMEOUT_FOR_TERMINAL_WORKER);
                    cplus_mempool_free(tp->worker_pool, worker);
                }
            }
        }
        else
        {
            count_to_change = worker_count - current_worker_count;
            while (count_to_change --)
            {
                cplus_task executor = NULL;
                struct task_worker * worker = NULL;
                CPLUS_TASK_CONFIG_T task_config = {0};

                task_config.proc = task_worker;
                task_config.param1 = tp;
                task_config.duration = PERIOD_FOR_WORKER_FREQUENCY;
                task_config.suspend = true;
                task_config.stacksize = old_stack_size;
                if (NULL == (executor = cplus_task_new_ex(&task_config)))
                {
                    break;
                }

                if ((worker = (struct task_worker *)cplus_mempool_alloc(tp->worker_pool)))
                {
                    worker->pid = cplus_task_get_pid(executor);
                    worker->executor = executor;
                }

                cplus_task_start(executor, 0);
                cplus_task_wait_start(executor, CPLUS_INFINITE_TIMEOUT);
                cplus_llist_push_front(tp->worker_list, worker);
            }
        }
        current_worker_count = cplus_llist_get_size(tp->worker_list);
    }
    cplus_crit_sect_exit(tp->worker_access_sect);

    return (current_worker_count == worker_count)? CPLUS_SUCCESS: CPLUS_FAIL;
}

uint32_t cplus_taskpool_get_task_count(cplus_taskpool obj)
{
    uint32_t count = 0;
    struct taskpool * tp = (struct taskpool *)obj;
    CHECK_OBJECT_TYPE(obj);

    cplus_crit_sect_enter(tp->task_access_sect);
    {
        count = cplus_llist_get_size(tp->task_list);
    }
    cplus_crit_sect_exit(tp->task_access_sect);

    return count;
}

int32_t cplus_taskpool_all_pause(cplus_taskpool obj, bool pause)
{
    struct taskpool * tp = (struct taskpool *)obj;
    struct task_worker * worker = NULL;
    CHECK_OBJECT_TYPE(obj);

    if (tp->worker_list)
    {
        cplus_crit_sect_enter(tp->worker_access_sect);
        CPLUS_LLIST_FOREACH(tp->worker_list, worker)
        {
            cplus_task_pause(worker->executor, pause);
        }
        cplus_crit_sect_exit(tp->worker_access_sect);
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_taskpool_clear_task(cplus_taskpool obj)
{
    struct taskpool * tp = (struct taskpool *)obj;
    CPLUS_TASKPOOL_TASK * task = NULL;
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(tp->task_list, CPLUS_FAIL);

    cplus_crit_sect_enter(tp->task_access_sect);
    {
        while (NULL != (task = cplus_llist_pop_back(tp->task_list)))
        {
            cplus_mempool_free(tp->task_pool, task);
        }
    }
    cplus_crit_sect_exit(tp->task_access_sect);

    return CPLUS_SUCCESS;
}

#ifdef __CPLUS_UNITTEST__
#include "cplus_atomic.h"
#include "cplus_pevent.h"
#include <pthread.h>

void acc_10(void * param1, void * param2)
{
    cplus_taskpool evt_start = (cplus_taskpool)param1;
    int32_t * result = (int32_t *)param2;

    while(0 == cplus_pevent_wait(evt_start, CPLUS_INFINITE_TIMEOUT))
    {
        cplus_atomic_add(result, 1);
        break;
    }
}

CPLUS_UNIT_TEST(cplus_taskpool_new_ex, functionity)
{
    cplus_taskpool taskpool = NULL;
    cplus_taskpool evt_start = NULL;
    CPLUS_TASKPOOL_CONFIG_T config = {0};
    CPLUS_TASKPOOL_TASK_T task = {0};
    int32_t result = 0;

    config.worker_count = 3;
    config.max_task_count = 10;
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new_ex(&config)));
    UNITTEST_EXPECT_EQ(true, (NULL != (evt_start = cplus_pevent_new(true, false))));

    for (int32_t i = 0; i < 10; i++)
    {
        task.proc = acc_10;
        task.param1 = evt_start;
        task.param2 = &result;
        task.callback = NULL;
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_add_task_ex(taskpool, &task));
    }

    cplus_systime_sleep_msec(1000);
    cplus_pevent_set(evt_start);
    cplus_systime_sleep_msec(2 * 1000);
    UNITTEST_EXPECT_EQ(10, result);

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(evt_start));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

#define TASK_COUNT 100
static int32_t test_value = TASK_COUNT;

void task_atomic_sub(void * param1, void * param2)
{
    cplus_atomic_add(&test_value, -1);
}

void * task_assign_task(void * args)
{
    cplus_taskpool taskpool = (cplus_taskpool)args;
    cplus_taskpool_add_task(taskpool, task_atomic_sub, NULL);
    return NULL;
}

CPLUS_UNIT_TEST(cplus_taskpool_add_task_ex, thread_safe)
{
    int inx = 0;
    cplus_taskpool taskpool = NULL;
    pthread_t tasks[TASK_COUNT];
    CPLUS_TASKPOOL_CONFIG_T config = {0};

    config.worker_count = 10;
    config.max_task_count = 255;
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = (cplus_taskpool)cplus_taskpool_new_ex(&config)));
    for (inx = 0; inx < TASK_COUNT; inx++)
    {
        pthread_create(&tasks[inx], NULL, task_assign_task, taskpool);
        cplus_systime_sleep_msec(2);
    }
    for (inx = 0; inx < TASK_COUNT; inx++)
    {
        pthread_join(tasks[inx], NULL);
    }
    UNITTEST_EXPECT_EQ(0, test_value);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_taskpool_get_worker_count, functionity)
{
    cplus_taskpool taskpool = NULL;

    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(10)));
    UNITTEST_EXPECT_EQ(10, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(9)));
    UNITTEST_EXPECT_EQ(9, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(7)));
    UNITTEST_EXPECT_EQ(7, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(5)));
    UNITTEST_EXPECT_EQ(5, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(3)));
    UNITTEST_EXPECT_EQ(3, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(1)));
    UNITTEST_EXPECT_EQ(1, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_taskpool_reset_worker_count, functionity)
{;
    cplus_taskpool taskpool = NULL;
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new(10)));
    UNITTEST_EXPECT_EQ(10, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 10));
    UNITTEST_EXPECT_EQ(10, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 5));
    UNITTEST_EXPECT_EQ(5, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 8));
    UNITTEST_EXPECT_EQ(8, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 1));
    UNITTEST_EXPECT_EQ(1, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 0));
    UNITTEST_EXPECT_EQ(0, cplus_taskpool_get_worker_count(taskpool));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void acc_proc(void * param1, void * param2)
{
    int32_t * count = (int32_t *)param1;
    UNUSED_PARAM(param2);
    cplus_atomic_add(count, 1);
}

CPLUS_UNIT_TEST(cplus_taskpool_all_pause, functionity)
{
    cplus_taskpool taskpool = NULL;
    CPLUS_TASKPOOL_CONFIG_T config = {0};
    int32_t count = 0;

    config.worker_count = 5;
    config.max_task_count = 255;
    config.get_task_cycling = true;
    UNITTEST_EXPECT_EQ(true, NULL != (taskpool = cplus_taskpool_new_ex(&config)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_all_pause(taskpool, true));
    for (int32_t i = 0; i < 10; i++)
    {
        cplus_taskpool_add_task(taskpool, acc_proc, &count);
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_all_pause(taskpool, false));
    while(1)
    {
        if (50 < cplus_atomic_read(&count))
        {
            cplus_taskpool_all_pause(taskpool, true);
            break;
        }
        cplus_systime_sleep_msec(10);
    }
    cplus_systime_sleep_msec(2000);
    UNITTEST_EXPECT_EQ(true, (count > 50));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_all_pause(taskpool, false));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete_ex(taskpool, 1000));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_taskpool(void)
{
    UNITTEST_ADD_TESTCASE(cplus_taskpool_new_ex, functionity);
    UNITTEST_ADD_TESTCASE(cplus_taskpool_add_task_ex, thread_safe);
    UNITTEST_ADD_TESTCASE(cplus_taskpool_get_worker_count, functionity);
    UNITTEST_ADD_TESTCASE(cplus_taskpool_reset_worker_count, functionity);
    UNITTEST_ADD_TESTCASE(cplus_taskpool_all_pause, functionity);
}

#endif // __CPLUS_UNITTEST__