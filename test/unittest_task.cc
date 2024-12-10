#include "internal.h"

void test_back(void * param1, void * param2)
{
	int32_t * time = (int32_t *)param1;
	EXPECT_EQ(1, 1 == *time);
}

void test_proc(void * param1, void * param2)
{
	int32_t * time = (int32_t *)param1;
	for (int32_t i = 0; i < *time; i++)
	{
		cplus_systime_sleep_msec(1000);
	}
}

TEST(cplus_task_oneshot_ex, functionity)
{
	cplus_task task = CPLUS_NULL;
	int32_t repect = 1;

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_oneshot_ex(test_proc, &repect, CPLUS_NULL, test_back)));
	cplus_systime_sleep_msec(1500);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_task_new, functionity)
{
	cplus_task task = CPLUS_NULL;
	uint32_t time, total_spent;
	int32_t repect = 5; // run 5 second
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_proc, &repect, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_start(task, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_start(task, 500));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 500));
	time = cplus_systime_elapsed_tick(time);
	total_spent = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_TRUE, time >= 500 && time < 550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	total_spent = cplus_systime_elapsed_tick(total_spent);
	EXPECT_EQ(CPLUS_TRUE, total_spent <= 5500);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_task_set_duration, functionity)
{
	cplus_task task = CPLUS_NULL;
	int32_t repect = 5; // run 5 second
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_proc, &repect, CPLUS_NULL, 100)));
	EXPECT_EQ(100, cplus_task_get_duration(task));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 500));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_set_duration(task, 500));
	EXPECT_EQ(500, cplus_task_get_duration(task));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, cplus_mgr_report());
}

void test_set_loop_duration_proc(void * param1, void * param2)
{
	EXPECT_EQ(100, cplus_task_get_loop_duration());
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_set_loop_duration(500));
	EXPECT_EQ(500, cplus_task_get_loop_duration());
}

TEST(cplus_task_set_loop_duration, functionity)
{
	cplus_task task = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_set_loop_duration_proc, CPLUS_NULL, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_finish(task, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(500, cplus_task_get_duration(task));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, cplus_mgr_report());
}

void test_set_loop_finish_proc(void * param1, void * param2)
{
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_set_loop_finish());
	while(1) {};
}

TEST(cplus_task_set_loop_finish, functionity)
{
	cplus_task task = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_set_loop_finish_proc, CPLUS_NULL, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_finish(task, 0));
	EXPECT_EQ(EAGAIN, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_task_wait_finish(task, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_finish(task, CPLUS_INFINITE_TIMEOUT));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_task_stop(task, 2000));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 2000 && time < 2500);
	EXPECT_EQ(0, cplus_mgr_report());
}

void test_get_loop_last_timestamp_proc(void * param1, void * param2)
{
	EXPECT_NE(0, cplus_task_get_loop_last_timestamp());
}

TEST(cplus_task_get_loop_last_timestamp, functionity)
{
	cplus_task task = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_get_loop_last_timestamp_proc, CPLUS_NULL, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
	cplus_systime_sleep_msec(1000);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, cplus_mgr_report());
}

void test_task_stop_and_wait_proc(void * param1, void * param2)
{
	int32_t * time = (int32_t *)param1;
	for (int32_t i = 0; i < *time; i++)
	{
		cplus_systime_sleep_msec(1000);
	}
}

TEST(cplus_task_stop, functionity)
{
	cplus_task task = CPLUS_NULL;
	int32_t repect = 3;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_task_stop_and_wait_proc, &repect, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
	time = cplus_systime_get_tick();
	cplus_systime_sleep_msec(1000);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 3000 && time < 3050);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_task_stop_and_wait_proc, &repect, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
	time = cplus_systime_get_tick();
	cplus_systime_sleep_msec(1000);
	EXPECT_EQ(CPLUS_FAIL, cplus_task_stop(task, 0));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(EBUSY, errno);
	EXPECT_EQ(CPLUS_TRUE, time >= 1000 && time < 1050);
	repect = 90;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_task_stop_and_wait_proc, &repect, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
	time = cplus_systime_get_tick();
	cplus_systime_sleep_msec(1000);
	EXPECT_EQ(CPLUS_FAIL, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 60000 && time < 61050);
	EXPECT_EQ(ETIMEDOUT, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}

void test_pause_proc(void * param1, void * param2)
{
	int32_t * count = (int32_t *)param1;
	UNUSED_PARAM(param2);
	(* count) += 1;
}

TEST(cplus_task_pause, functionity)
{
	cplus_task task = CPLUS_NULL;
	int32_t count = 0;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (task = cplus_task_new(test_pause_proc, &count, CPLUS_NULL, 100)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_start(task, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_wait_start(task, CPLUS_INFINITE_TIMEOUT));
	while (1)
	{
		if (5 == count)
		{
			EXPECT_EQ(CPLUS_SUCCESS, cplus_task_pause(task, true));
			break;
		}
	}
	cplus_systime_sleep_msec(2000);
	EXPECT_EQ(5, count);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_pause(task, false));
	while (1)
	{
		if (10 == count)
		{
			EXPECT_EQ(10, count);
			EXPECT_EQ(CPLUS_SUCCESS, cplus_task_pause(task, true));
			break;
		}
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_task_stop(task, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, cplus_mgr_report());
}