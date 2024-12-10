#include "internal.h"
#include <linux/sem.h>
#include <semaphore.h>
#define SEMAPHORE_MAX_VALUE SEMVMX
static char TEST_SEMAPHORE_NAME[] = "test";

TEST(cplus_semaphore_new, functionity)
{
	cplus_semaphore semp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp = cplus_semaphore_new(10))));
	EXPECT_EQ(10, cplus_semaphore_get_value(semp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_push, functionity)
{
	cplus_semaphore semp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp = cplus_semaphore_new(10))));
	EXPECT_EQ(10, cplus_semaphore_get_value(semp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp, 10));
	EXPECT_EQ(20, cplus_semaphore_get_value(semp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp, 20));
	EXPECT_EQ(40, cplus_semaphore_get_value(semp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_wait_poll, functionity)
{
	cplus_semaphore semp = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp = cplus_semaphore_new(10))));
	for (int32_t i = 10; i > 0; i--)
	{
		EXPECT_EQ(i, cplus_semaphore_get_value(semp));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp, 0));
	}
	EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp, 0));
	EXPECT_EQ(EAGAIN, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp, 1500));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_new, bad_parameter)
{
	cplus_semaphore semp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp = cplus_semaphore_new(SEMAPHORE_MAX_VALUE + 1))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp = cplus_semaphore_new(-111))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_push, bad_parameter)
{
	cplus_semaphore semp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp = cplus_semaphore_new(10))));
	EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_push(semp, SEMAPHORE_MAX_VALUE - 9));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp, SEMAPHORE_MAX_VALUE - 10));
	EXPECT_EQ(SEMAPHORE_MAX_VALUE, cplus_semaphore_get_value(semp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_wait_poll, bad_parameter)
{
	cplus_semaphore semp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp = cplus_semaphore_new(10))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_new_xp, cross_process_test)
{
	cplus_semaphore semp_sv = CPLUS_NULL, semp_ch = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp_sv = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, 10))));
	EXPECT_EQ(10, cplus_semaphore_get_value(semp_sv));
	if (0 != fork())
	{
	}
	else
	{
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp_ch = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, 1))));
		EXPECT_EQ(10, cplus_semaphore_get_value(semp_ch));
		for (int32_t i = 10; i > 0; i--)
		{
			EXPECT_EQ(i, cplus_semaphore_get_value(semp_ch));
			EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp_ch, 0));
		}
		EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_create_xp, cross_process_test)
{
	cplus_semaphore semp_sv = CPLUS_NULL, semp_ch = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp_sv = cplus_semaphore_create_xp(TEST_SEMAPHORE_NAME, 10))));
	EXPECT_EQ(10, cplus_semaphore_get_value(semp_sv));
	for (int32_t i = 10; i > 0; i--)
	{
		EXPECT_EQ(i, cplus_semaphore_get_value(semp_sv));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp_sv, 0));
	}
	EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp_sv, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp_sv, 1));
	if (0 != fork())
	{
	}
	else
	{
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp_ch = cplus_semaphore_open_xp(TEST_SEMAPHORE_NAME))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp_ch, CPLUS_INFINITE_TIMEOUT));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_new_xp, bad_parameter)
{
	cplus_semaphore semp_sv = CPLUS_NULL;
	char bad_name[23] = "0123456789abcdef012345";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_new_xp(CPLUS_NULL, 10))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_new_xp(bad_name, 10))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp_sv = cplus_semaphore_new_xp(&bad_name[1], 10))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, SEMAPHORE_MAX_VALUE + 1))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, -45465))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_create_xp, bad_parameter)
{
	cplus_semaphore semp_sv = CPLUS_NULL, semp_ch = CPLUS_NULL;
	char bad_name[23] = "0123456789abcdef012345";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_create_xp(CPLUS_NULL, 10))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_create_xp(bad_name, 10))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (semp_sv = cplus_semaphore_create_xp(&bad_name[3], 10))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_ch = cplus_semaphore_create_xp(&bad_name[3], 10))));
	EXPECT_EQ(EEXIST, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_create_xp(TEST_SEMAPHORE_NAME, SEMAPHORE_MAX_VALUE + 1))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_create_xp(TEST_SEMAPHORE_NAME, -45465))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_semaphore_open_xp, bad_parameter)
{
	cplus_semaphore semp_sv = CPLUS_NULL;
	char bad_name[23] = "0123456789abcdef012345";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_open_xp(CPLUS_NULL))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_open_xp(bad_name))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (semp_sv = cplus_semaphore_open_xp(&bad_name[3]))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}