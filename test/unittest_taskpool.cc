#include "internal.h"

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

TEST(cplus_taskpool_new_ex, functionity)
{
	cplus_taskpool taskpool = CPLUS_NULL;
	cplus_taskpool evt_start = CPLUS_NULL;
	struct cplus_taskpool_config config;
	struct cplus_taskpool_task task;
	int32_t result = 0;
	CPLUS_MEM_CLEAN(&(config));
	config.worker_count = 3;
	config.max_task_count = 10;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new_ex(&config)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (evt_start = cplus_pevent_new(true, false))));
	CPLUS_MEM_CLEAN(&(task));
	for (int32_t i = 0; i < 10; i++)
	{
		task.proc = acc_10;
		task.param1 = evt_start;
		task.param2 = &result;
		task.callback = CPLUS_NULL;
		EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_add_task_ex(taskpool, &task));
	}
	cplus_systime_sleep_msec(1000);
	cplus_pevent_set(evt_start);
	cplus_systime_sleep_msec(2 * 1000);
	EXPECT_EQ(10, result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(evt_start));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(0, cplus_mgr_report());
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
	cplus_taskpool_add_task(taskpool, task_atomic_sub, CPLUS_NULL);
	return CPLUS_NULL;
}

TEST(cplus_taskpool_add_task_ex, thread_safe)
{
	int inx = 0;
	cplus_taskpool taskpool = CPLUS_NULL;
	pthread_t tasks[TASK_COUNT];
	struct cplus_taskpool_config config;
	CPLUS_MEM_CLEAN(&(config));
	config.worker_count = 10;
	config.max_task_count = 255;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = (cplus_taskpool)cplus_taskpool_new_ex(&config)));
	for (inx = 0; inx < TASK_COUNT; inx++)
	{
		pthread_create(&tasks[inx], CPLUS_NULL, task_assign_task, taskpool);
		cplus_systime_sleep_msec(2);
	}
	for (inx = 0; inx < TASK_COUNT; inx++)
	{
		pthread_join(tasks[inx], CPLUS_NULL);
	}
	EXPECT_EQ(0, test_value);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_taskpool_get_worker_count, functionity)
{
	cplus_taskpool taskpool = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(10)));
	EXPECT_EQ(10, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(9)));
	EXPECT_EQ(9, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(7)));
	EXPECT_EQ(7, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(5)));
	EXPECT_EQ(5, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(3)));
	EXPECT_EQ(3, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(1)));
	EXPECT_EQ(1, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_taskpool_reset_worker_count, functionity)
{
	cplus_taskpool taskpool = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new(10)));
	EXPECT_EQ(10, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 10));
	EXPECT_EQ(10, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 5));
	EXPECT_EQ(5, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 8));
	EXPECT_EQ(8, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 1));
	EXPECT_EQ(1, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_reset_worker_count(taskpool, 0));
	EXPECT_EQ(0, cplus_taskpool_get_worker_count(taskpool));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete(taskpool));
	EXPECT_EQ(0, cplus_mgr_report());
}

void acc_proc(void * param1, void * param2)
{
	int32_t * count = (int32_t *)param1;
	UNUSED_PARAM(param2);
	cplus_atomic_add(count, 1);
}

TEST(cplus_taskpool_all_pause, functionity)
{
	cplus_taskpool taskpool = CPLUS_NULL;
	struct cplus_taskpool_config config;
	int32_t count = 0;
	CPLUS_MEM_CLEAN(&(config));
	config.worker_count = 5;
	config.max_task_count = 255;
	config.get_task_cycling = true;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (taskpool = cplus_taskpool_new_ex(&config)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_all_pause(taskpool, true));
	for (int32_t i = 0; i < 10; i++)
	{
		cplus_taskpool_add_task(taskpool, acc_proc, &count);
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_all_pause(taskpool, false));
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
	EXPECT_EQ(CPLUS_TRUE, (count > 50));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_all_pause(taskpool, false));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_taskpool_delete_ex(taskpool, 1000));
	EXPECT_EQ(0, cplus_mgr_report());
}