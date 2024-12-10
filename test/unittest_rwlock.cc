#include "internal.h"
static char TEST_RWLOCK_NAME[] = "test";

TEST(cplus_rwlock_new, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_new_xp, cross_process_test)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_wrlock, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 0));
	EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 1500));
	EXPECT_EQ(EDEADLK, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1500);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_wrlock, cross_process_test)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 0));
		EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_ch, 0));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		EXPECT_EQ(EDEADLK, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_rdlock, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 0));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1500);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_sv, 0));
	EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_sv, 1500));
	EXPECT_EQ(EDEADLK, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1500);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_rdlock, cross_process_test)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 0));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1500);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_ch, 0));
		EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_ch, 0));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_new_xp, bad_parameter)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (lock_sv = cplus_rwlock_new_xp(bad_name))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(&bad_name[1]))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_create_xp, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_create_xp(TEST_RWLOCK_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_ch = cplus_rwlock_open_xp(TEST_RWLOCK_NAME))));
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 0));
		EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_ch = cplus_rwlock_create_xp(TEST_RWLOCK_NAME))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_ch, 0));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		EXPECT_EQ(EDEADLK, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_rwlock_wrlock, PREFER_WRITER)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	struct cplus_rwlock_config config;
	CPLUS_MEM_CLEAN(&(config));
	config.kind = CPLUS_RWLOCK_KIND_PREFER_WRITER_NONRECURSIVE;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_config(&config))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 0));
	EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 1500));
	EXPECT_EQ(EDEADLK, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1500);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}