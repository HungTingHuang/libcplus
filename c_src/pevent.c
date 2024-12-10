/******************************************************************
* @file: pevent.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <pthread.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_systime.h"
#include "cplus_sharedmem.h"
#include "cplus_pevent.h"

#define OBJ_TYPE (OBJ_NONE + CTRL + 0)
#define PEVENT_NAME_MAX_SIZE 31
#define PEVENT_NAME_PATTERN	"pevent.%s"
#define PEVENT_NAME_PATTERN_SIZE (sizeof(PEVENT_NAME_PATTERN) - 3)

struct named_pevent
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool setted;
	bool broadcast;
	bool status;
};

struct pevent
{
	uint16_t type;
	bool * ptr_setted;
	bool * ptr_broadcast;
	bool * ptr_status;
	pthread_mutex_t * ptr_mutex;
	pthread_cond_t * ptr_cond;
	bool setted;
	bool broadcast;
	bool status;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	cplus_sharedmem shared_mem;
};

static int32_t unlock_and_wait_cond(struct pevent * evt, uint32_t timeout)
{
	int32_t res = 0;
	struct timespec ts;

	CPLUS_MEM_CLEAN(&(ts));
	if (false == (*(evt->ptr_setted)))
	{
		if (0 == timeout)
		{
			errno = EAGAIN;
			return -1;
		}

		do
		{
			if (CPLUS_INFINITE_TIMEOUT == timeout)
			{
				if (0 != (res = pthread_cond_wait(evt->ptr_cond, evt->ptr_mutex)))
				{
					errno = res;
					res = -1;
				}
			}
			else
			{
				cplus_systime_get_abstick_after_msec(&ts, timeout);
				if (0 != (res = pthread_cond_timedwait(evt->ptr_cond, evt->ptr_mutex, &ts)))
				{
					errno = res;
					res = -1;
				}
			}
		}
		while (0 == res AND false == (* (evt->ptr_setted)));

		if (0 == res AND false == (* (evt->ptr_broadcast)))
		{
			(* (evt->ptr_setted)) = false;
		}
	}
	else
	{
		if (false == (* (evt->ptr_broadcast)))
		{
			(* (evt->ptr_setted)) = false;
		}
	}

	return res;
}

int32_t cplus_pevent_wait(cplus_pevent obj, uint32_t timeout)
{
	struct pevent * evt = (struct pevent *)(obj);
	CHECK_OBJECT_TYPE(obj);

	if (0 == timeout)
	{
		if (0 != (errno = pthread_mutex_trylock(evt->ptr_mutex)))
		{
			return CPLUS_FAIL;
		}
	}
	else
	{
		if (0 != (errno = pthread_mutex_lock(evt->ptr_mutex)))
		{
			return CPLUS_FAIL;
		}
	}

	if (0 != unlock_and_wait_cond(evt, timeout))
	{
		pthread_mutex_unlock(evt->ptr_mutex);
		return CPLUS_FAIL;
	}

	if (0 != (errno = pthread_mutex_unlock(evt->ptr_mutex)))
	{
		return CPLUS_FAIL;
	}

	return CPLUS_SUCCESS;
}

int32_t cplus_pevent_reset(cplus_pevent obj)
{
	int32_t res = CPLUS_SUCCESS;
	struct pevent * evt = (struct pevent *)(obj);

	CHECK_OBJECT_TYPE(obj);

	res = pthread_mutex_lock(evt->ptr_mutex);
	if (0 == res)
	{
		(* (evt->ptr_setted)) = false;
		res = pthread_mutex_unlock(evt->ptr_mutex);
	}
	else
	{
		errno = res;
		res = CPLUS_FAIL;
	}

	return res;
}

int32_t cplus_pevent_set(cplus_pevent obj)
{
	int32_t res = CPLUS_SUCCESS;
	struct pevent * evt = (struct pevent *)(obj);

	CHECK_OBJECT_TYPE(obj);

	res = pthread_mutex_lock(evt->ptr_mutex);
	if (0 == res)
	{
		(* (evt->ptr_setted)) = true;

		if ((* (evt->ptr_broadcast)))
		{
			res = pthread_cond_broadcast(evt->ptr_cond);
		}
		else
		{
			res = pthread_cond_signal(evt->ptr_cond);
		}

		pthread_mutex_unlock(evt->ptr_mutex);
	}
	else
	{
		errno = res;
		res = CPLUS_FAIL;
	}

	return res;
}

bool cplus_pevent_get_status(cplus_pevent obj)
{
	bool setted = false;
	struct pevent * evt = (struct pevent *)(obj);

	CHECK_OBJECT_TYPE(obj);

	if (0 == pthread_mutex_lock(evt->ptr_mutex))
	{
		setted = (* (evt->ptr_setted));
		pthread_mutex_unlock(evt->ptr_mutex);
	}

	return setted;
}

int32_t cplus_pevent_delete(cplus_pevent obj)
{
	struct pevent * evt = (struct pevent *)(obj);
	CHECK_OBJECT_TYPE(obj);

	if (evt->shared_mem)
	{
		if (true == cplus_sharedmem_is_owner(evt->shared_mem))
		{
			if (evt->ptr_mutex)
			{
				pthread_mutex_destroy(evt->ptr_mutex);
			}
			if (evt->ptr_cond)
			{
				pthread_cond_destroy(evt->ptr_cond);
			}
		}
		cplus_sharedmem_delete(evt->shared_mem);
	}
	else
	{
		if (evt->ptr_mutex)
		{
			pthread_mutex_destroy(evt->ptr_mutex);
		}
		if (evt->ptr_cond)
		{
			pthread_cond_destroy(evt->ptr_cond);
		}
	}

	cplus_free(evt);
	return CPLUS_SUCCESS;
}

static void * pevent_initialize_object(
	const char * name
	, bool broadcast
	, bool status
	, enum INIT_MODE mode)
{
	struct pevent * evt = CPLUS_NULL;
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;

	if ((evt = (struct pevent * )cplus_malloc(sizeof(struct pevent))))
	{
		CPLUS_INITIALIZE_STRUCT_POINTER(evt);
		evt->type = OBJ_TYPE;
		evt->ptr_setted = CPLUS_NULL;
		evt->ptr_broadcast = CPLUS_NULL;
		evt->ptr_status = CPLUS_NULL;
		evt->ptr_mutex = CPLUS_NULL;
		evt->ptr_cond = CPLUS_NULL;
		evt->shared_mem = CPLUS_NULL;

		if (CPLUS_NULL == name)
		{
			if (0 != pthread_mutex_init(&(evt->mutex), CPLUS_NULL))
			{
				goto exit;
			}

			if (0 != pthread_condattr_init(&(cond_attr)))
			{
				goto exit;
			}

			if (0 != pthread_condattr_setclock(&(cond_attr), CLOCK_MONOTONIC)
				OR 0 != pthread_cond_init(&(evt->cond), &(cond_attr)))
			{
				pthread_condattr_destroy(&(cond_attr));
				goto exit;
			}
			pthread_condattr_destroy(&(cond_attr));

			evt->setted = false;
			evt->broadcast = broadcast;
			evt->status = status;

			evt->ptr_setted = &(evt->setted);
			evt->ptr_broadcast = &(evt->broadcast);
			evt->ptr_status = &(evt->status);
			evt->ptr_mutex = &(evt->mutex);
			evt->ptr_cond = &(evt->cond);
		}
		else
		{
			char pevent_name[PEVENT_NAME_MAX_SIZE + 1] = {0};

			if (0 > cplus_str_printf(
				pevent_name
				, PEVENT_NAME_MAX_SIZE
				, PEVENT_NAME_PATTERN
				, name))
			{
				goto exit;
			}

			switch(mode)
			{
			case INIT_CREATE:
				{
					evt->shared_mem = cplus_sharedmem_create(pevent_name
						, sizeof(struct named_pevent));
				}
				break;
			case INIT_OPEN:
				{
					evt->shared_mem = cplus_sharedmem_open(pevent_name);
				}
				break;
			case INIT_HYBRID:
				{
					evt->shared_mem = cplus_sharedmem_new(pevent_name
						, sizeof(struct named_pevent));
				}
				break;
			case INIT_NONE:
			default:
				errno = EINVAL;
				goto exit;
			}

			if (CPLUS_NULL == evt->shared_mem)
			{
				goto exit;
			}
			else
			{
				struct named_pevent * shared_pevent = (struct named_pevent *)cplus_sharedmem_alloc(evt->shared_mem);

				if (true == cplus_sharedmem_is_owner(evt->shared_mem))
				{
					if (0 != pthread_mutexattr_init(&(mutex_attr)))
					{
						goto exit;
					}

					if (0 != pthread_mutexattr_setpshared(&(mutex_attr), PTHREAD_PROCESS_SHARED)
						OR 0 != pthread_mutex_init(&(shared_pevent->mutex), &(mutex_attr)))
					{
						pthread_mutexattr_destroy(&(mutex_attr));
						goto exit;
					}
					pthread_mutexattr_destroy(&(mutex_attr));

					if (0 != pthread_condattr_init(&(cond_attr)))
					{
						goto exit;
					}

					if (0 != pthread_condattr_setclock(&(cond_attr), CLOCK_MONOTONIC)
						OR 0 != pthread_condattr_setpshared(&(cond_attr), PTHREAD_PROCESS_SHARED)
						OR 0 != pthread_cond_init(&(shared_pevent->cond), &(cond_attr)))
					{
						pthread_condattr_destroy(&(cond_attr));
						goto exit;
					}
					pthread_condattr_destroy(&(cond_attr));

					shared_pevent->setted = false;
					shared_pevent->broadcast = broadcast;
					shared_pevent->status = status;
				}

				evt->ptr_setted = &(shared_pevent->setted);
				evt->ptr_broadcast = &(shared_pevent->broadcast);
				evt->ptr_status = &(shared_pevent->status);
				evt->ptr_mutex = &(shared_pevent->mutex);
				evt->ptr_cond = &(shared_pevent->cond);
			}
		}

		if (true == status)
		{
			cplus_pevent_set(evt);
		}
	}
	return evt;
exit:
	cplus_pevent_delete(evt);
	return CPLUS_NULL;
}

cplus_pevent cplus_pevent_new(bool broadcast, bool status)
{
	return pevent_initialize_object(CPLUS_NULL, !!broadcast, !!status, INIT_NONE);
}

cplus_pevent cplus_pevent_new_xp(const char * name, bool broadcast, bool status)
{
	CHECK_IF(CPLUS_NULL == name, CPLUS_NULL);
	CHECK_IF(PEVENT_NAME_MAX_SIZE < (strlen(name) + PEVENT_NAME_PATTERN_SIZE), CPLUS_NULL);
	return pevent_initialize_object(name, !!broadcast, !!status, INIT_HYBRID);
}

cplus_pevent cplus_pevent_create_xp(const char * name, bool broadcast, bool status)
{
	CHECK_IF(CPLUS_NULL == name, CPLUS_NULL);
	CHECK_IF(PEVENT_NAME_MAX_SIZE < (strlen(name) + PEVENT_NAME_PATTERN_SIZE), CPLUS_NULL);
	return pevent_initialize_object(name, !!broadcast, !!status, INIT_CREATE);
}

cplus_pevent cplus_pevent_open_xp(const char * name)
{
	CHECK_IF(CPLUS_NULL == name, CPLUS_NULL);
	CHECK_IF(PEVENT_NAME_MAX_SIZE < (strlen(name) + PEVENT_NAME_PATTERN_SIZE), CPLUS_NULL);
	return pevent_initialize_object(name, 0, 0, INIT_OPEN);
}

bool cplus_pevent_check(cplus_object obj)
{
	return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#ifdef __CPLUS_UNITTEST__
static char TEST_PEVENT_NAME[] = "test";

CPLUS_UNIT_TEST(cplus_pevent_new, functionity)
{
	cplus_pevent event_sv = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, false))));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(EAGAIN, errno);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(EAGAIN, errno);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, true))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(EAGAIN, errno);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, false))));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(EAGAIN, errno);
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(EAGAIN, errno);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, true))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 0));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_wait, functionity)
{
	cplus_pevent event_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, false))));
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 1500));
	UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(false, true))));
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1550);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, false))));
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 1500));
	UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_sv, 1500));
	UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));

	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1550);
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1550);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new(true, true))));
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1550);
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1550);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_reset, functionity)
{
	cplus_pevent event = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event = cplus_pevent_new(false, true))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event, 0));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event, 0));
	UNITTEST_EXPECT_EQ(EAGAIN, errno);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event));

	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}


CPLUS_UNIT_TEST(cplus_pevent_new_xp, cross_process_test)
{
	cplus_pevent event_sv = CPLUS_NULL;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(EAGAIN, errno);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(EAGAIN, errno);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, true))));
	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(EAGAIN, errno);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, true, false))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(EAGAIN, errno);
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(EAGAIN, errno);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(TEST_PEVENT_NAME, true, true))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, false))));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_wait, cross_process_test)
{
	cplus_pevent event_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, false, false))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 1500));
		UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, false, true))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;
		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, true, false))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 1500));
		UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 1500));
		UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event_sv));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, true, true))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_reset, cross_process_test)
{
	cplus_pevent event_sv = CPLUS_NULL;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(TEST_PEVENT_NAME, false, true))));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event_sv));

	if (0 != fork())
	{
	}
	else
	{
		cplus_pevent event_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_pevent_wait(event_ch, 0));
		UNITTEST_EXPECT_EQ(EAGAIN, errno);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_new_xp, bad_parameter)
{
	cplus_pevent event_sv = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL == (event_sv = cplus_pevent_new_xp(bad_name, false, true))));
	UNITTEST_EXPECT_EQ(EINVAL, errno);

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_new_xp(&bad_name[1], false, true))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_create_xp, bad_parameter)
{
	cplus_pevent event_sv = CPLUS_NULL, event_ch = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL == (event_sv = cplus_pevent_create_xp(bad_name, false, true))));
	UNITTEST_EXPECT_EQ(EINVAL, errno);
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(&bad_name[1], false, true))));
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL == (event_ch = cplus_pevent_create_xp(&bad_name[1], false, true))));
	UNITTEST_EXPECT_EQ(EEXIST, errno);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_open_xp, bad_parameter)
{
	cplus_pevent event_sv = CPLUS_NULL, event_ch = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL == (event_sv = cplus_pevent_open_xp(bad_name))));
	UNITTEST_EXPECT_EQ(EINVAL, errno);
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL == (event_ch = cplus_pevent_open_xp(TEST_PEVENT_NAME))));
	UNITTEST_EXPECT_EQ(ENOENT, errno);
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_sv = cplus_pevent_create_xp(&bad_name[1], false, true))));
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event_ch = cplus_pevent_open_xp(&bad_name[1]))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event_ch));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_get_status, functionity)
{
	cplus_pevent event = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event = cplus_pevent_new(false, false))));
	UNITTEST_EXPECT_EQ(false, cplus_pevent_get_status(event));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event));
	UNITTEST_EXPECT_EQ(true, cplus_pevent_get_status(event));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event));
	UNITTEST_EXPECT_EQ(false, cplus_pevent_get_status(event));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_pevent_get_status, cross_process_test)
{
	cplus_pevent event = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (event = cplus_pevent_new_xp(TEST_PEVENT_NAME, false, true))));
	UNITTEST_EXPECT_EQ(true, cplus_pevent_get_status(event));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_reset(event));
	UNITTEST_EXPECT_EQ(false, cplus_pevent_get_status(event));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_set(event));
	UNITTEST_EXPECT_EQ(true, cplus_pevent_get_status(event));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_pevent_delete(event));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_pevent(void)
{
	UNITTEST_ADD_TESTCASE(cplus_pevent_new, functionity);
	UNITTEST_ADD_TESTCASE(cplus_pevent_wait, functionity);
	UNITTEST_ADD_TESTCASE(cplus_pevent_reset, functionity);
	UNITTEST_ADD_TESTCASE(cplus_pevent_get_status, functionity);
	UNITTEST_ADD_TESTCASE(cplus_pevent_new_xp, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_pevent_wait, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_pevent_reset, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_pevent_get_status, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_pevent_new_xp, bad_parameter);
	UNITTEST_ADD_TESTCASE(cplus_pevent_create_xp, bad_parameter);
	UNITTEST_ADD_TESTCASE(cplus_pevent_open_xp, bad_parameter);
}

#endif // __CPLUS_UNITTEST__