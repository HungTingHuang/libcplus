#include "internal.h"
static char TEST_PEVENT_NAME[] = "test";

TEST(cplus_pevent_new, functionity)
{
	cplus_pevent event_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, false))));
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, true))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, false))));
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, true))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_wait, functionity)
{
	cplus_pevent event_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, false))));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 1500));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, true))));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, false))));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 1500));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 1500));
	EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1550);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, true))));
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1550);
	time = cplus_systime_get_tick();
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	EXPECT_EQ(CPLUS_TRUE, time < 1550);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_reset, functionity)
{
	cplus_pevent event = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event = cplus_pevent_new(false, true))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event));
	EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event, 0));
	EXPECT_EQ(EAGAIN, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_new_xp, cross_process_test)
{
	cplus_pevent event_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, true))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, true, false))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, true, true))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_wait, cross_process_test)
{
	cplus_pevent event_sv = CPLUS_NULL;
	uint32_t time;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, false, false))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, false, true))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, true, false))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 1500));
		EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time >= 1500 && time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, true, true))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		time = cplus_systime_get_tick();
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		EXPECT_EQ(CPLUS_TRUE, time < 1550);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_reset, cross_process_test)
{
	cplus_pevent event_sv = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, false, true))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event_sv));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		EXPECT_EQ(EAGAIN, errno);
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		EXPECT_EQ(0, cplus_mgr_report());
		return;
	}
	EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_new_xp, bad_parameter)
{
	cplus_pevent event_sv = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (event_sv = cplus_pevent_new_xp(bad_name, false, true))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(&bad_name[1], false, true))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_create_xp, bad_parameter)
{
	cplus_pevent event_sv = CPLUS_NULL, event_ch = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (event_sv = cplus_pevent_create_xp(bad_name, false, true))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(&bad_name[1], false, true))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (event_ch = cplus_pevent_create_xp(&bad_name[1], false, true))));
	EXPECT_EQ(EEXIST, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_open_xp, bad_parameter)
{
	cplus_pevent event_sv = CPLUS_NULL, event_ch = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (event_sv = cplus_pevent_open_xp(bad_name))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(&bad_name[1], false, true))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(&bad_name[1]))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_get_status, functionity)
{
	cplus_pevent event = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event = cplus_pevent_new(false, false))));
	EXPECT_EQ(CPLUS_FALSE, cplus_pevent_get_status(event));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event));
	EXPECT_EQ(CPLUS_TRUE, cplus_pevent_get_status(event));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event));
	EXPECT_EQ(CPLUS_FALSE, cplus_pevent_get_status(event));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_pevent_get_status, cross_process_test)
{
	cplus_pevent event = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (event = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, true))));
	EXPECT_EQ(CPLUS_TRUE, cplus_pevent_get_status(event));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event));
	EXPECT_EQ(CPLUS_FALSE, cplus_pevent_get_status(event));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event));
	EXPECT_EQ(CPLUS_TRUE, cplus_pevent_get_status(event));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event));
	EXPECT_EQ(0, cplus_mgr_report());
}