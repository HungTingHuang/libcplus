#include "internal.h"
static char TEST_MUTEX_NAME[] = "test";

TEST(cplus_mutex_new, functionity)
{
	cplus_mutex mtx = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx = cplus_mutex_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mutex_lock, functionity)
{
	cplus_mutex mtx = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx = cplus_mutex_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_lock(mtx, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx, 0));
	EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx, 1500));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_unlock(mtx));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
	EXPECT_EQ(0, cplus_mgr_report());
}


TEST(cplus_mutex_new_xp, cross_process_test)
{
	cplus_mutex mtx_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx_sv = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_lock(mtx_sv, CPLUS_INFINITE_TIMEOUT));
	if (0 != fork())
	{
	}
	else
	{
		cplus_mutex mtx_ch = CPLUS_NULL;
		uint32_t time;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx_ch = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
		EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 0));
		EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mutex_create_xp, cross_process_test)
{
	cplus_mutex mtx_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx_sv = cplus_mutex_create_xp(TEST_MUTEX_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_lock(mtx_sv, CPLUS_INFINITE_TIMEOUT));
	if (0 != fork())
	{
	}
	else
	{
		cplus_mutex mtx_ch = CPLUS_NULL;
		uint32_t time;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx_ch = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
		EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 0));
		EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mutex_new_xp, bad_parameter)
{
	cplus_mutex mtx = CPLUS_NULL;
	char bad_name[27] = "0123456789abcdef0123456789";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mtx = cplus_mutex_new_xp(bad_name))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx = cplus_mutex_new_xp(&bad_name[1]))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mutex_create_xp, bad_parameter)
{
	cplus_mutex mtx = CPLUS_NULL;
	char bad_name[27] = "0123456789abcdef0123456789";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mtx = cplus_mutex_create_xp(bad_name))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx = cplus_mutex_create_xp(&bad_name[1]))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mutex_open_xp, bad_parameter)
{
	cplus_mutex mtx = CPLUS_NULL;
	char bad_name[27] = "0123456789abcdef0123456789";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mtx = cplus_mutex_open_xp(bad_name))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mtx = cplus_mutex_open_xp(&bad_name[1]))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_crit_sect_enter, functionity)
{
	cplus_mutex mtx = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx = cplus_mutex_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_crit_sect_enter, cross_process_test)
{
	cplus_mutex mtx_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx_sv = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx_sv));
	if (0 != fork())
	{
	}
	else
	{
		cplus_mutex mtx_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mtx_ch = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}