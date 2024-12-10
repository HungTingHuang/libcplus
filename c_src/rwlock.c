/******************************************************************
* @file: rwlock.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <pthread.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_systime.h"
#include "cplus_sharedmem.h"
#include "cplus_rwlock.h"

#define OBJ_TYPE (OBJ_NONE + CTRL + 1)
#define RWLOCK_NAME_MAX_SIZE 31
#define RWLOCK_NAME_PATTERN	"rwlock.%s"
#define RWLOCK_NAME_PATTERN_SIZE (sizeof(RWLOCK_NAME_PATTERN) - 3)

struct named_rwlock
{
	pthread_rwlock_t rwlock;
};

struct rwlock
{
	uint16_t type;
	pthread_rwlock_t * rwlock;
	bool st_locked;
	uint32_t count_rdlocked;
	cplus_sharedmem shared_mem;
	pthread_rwlock_t nonamed_rwlock;
};

static int32_t convert_rwlock_kind(CPLUS_RWLOCK_KIND kind)
{
	switch (kind)
	{
	default:
	case CPLUS_RWLOCK_KIND_PREFER_READER:
		return PTHREAD_RWLOCK_PREFER_READER_NP;
	case CPLUS_RWLOCK_KIND_PREFER_WRITER:
		return PTHREAD_RWLOCK_PREFER_WRITER_NP;
	case CPLUS_RWLOCK_KIND_PREFER_WRITER_NONRECURSIVE:
		return PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP;
	}
}

int32_t cplus_rwlock_unlock(cplus_rwlock obj)
{
	int32_t res = CPLUS_FAIL;
	struct rwlock * lock = (struct rwlock *)(obj);

	CHECK_OBJECT_TYPE(obj);

	if (true == lock->st_locked)
	{
		res = pthread_rwlock_unlock(lock->rwlock);
		if (0 == res)
		{
			if (0 == lock->count_rdlocked)
			{
				lock->st_locked = false;
			}
			else
			{
				lock->count_rdlocked -= 1;
				if (0 == lock->count_rdlocked)
				{
					lock->st_locked = false;
				}
			}
			res = CPLUS_SUCCESS;
		}
	}

	return res;
}

int32_t cplus_rwlock_wrlock(cplus_rwlock obj, uint32_t timeout)
{
	int32_t res = CPLUS_FAIL;
	struct rwlock * lock = (struct rwlock *)(obj);
	struct timespec ts;
	CHECK_OBJECT_TYPE(obj);

	if (CPLUS_INFINITE_TIMEOUT == timeout)
	{
		res = pthread_rwlock_wrlock(lock->rwlock);
	}
	else if (0 == timeout)
	{
		res = pthread_rwlock_trywrlock(lock->rwlock);
	}
	else
	{
		CPLUS_MEM_CLEAN(&(ts));
		cplus_systime_get_abstime_after_msec(&(ts), timeout);
		res = pthread_rwlock_timedwrlock(lock->rwlock, &(ts));
	}

	if (0 != res)
	{
		errno = res;
		res = CPLUS_FAIL;
	}
	else
	{
		lock->st_locked = true;
		res = CPLUS_SUCCESS;
	}

	return res;

}

int32_t cplus_rwlock_rdlock(cplus_rwlock obj, uint32_t timeout)
{
	int32_t res = CPLUS_FAIL;
	struct rwlock * lock = (struct rwlock *)(obj);
	struct timespec ts;
	CHECK_OBJECT_TYPE(obj);

	if (CPLUS_INFINITE_TIMEOUT == timeout)
	{
		res = pthread_rwlock_rdlock(lock->rwlock);
	}
	else if (0 == timeout)
	{
		res = pthread_rwlock_tryrdlock(lock->rwlock);
	}
	else
	{
		CPLUS_MEM_CLEAN(&(ts));
		cplus_systime_get_abstime_after_msec(&(ts), timeout);
		res = pthread_rwlock_timedrdlock(lock->rwlock, &(ts));
	}

	if (0 != res)
	{
		errno = res;
		res = CPLUS_FAIL;
	}
	else
	{
		lock->st_locked = true;
		lock->count_rdlocked += 1;
		res = CPLUS_SUCCESS;
	}

	return res;
}

int32_t cplus_rwlock_delete(cplus_rwlock obj)
{
	struct rwlock * lock = (struct rwlock *)(obj);
	CHECK_OBJECT_TYPE(obj);

	if (lock->shared_mem)
	{
		if (cplus_sharedmem_is_owner(lock->shared_mem))
		{
			if (lock->rwlock)
			{
				pthread_rwlock_destroy(lock->rwlock);
			}
		}
		cplus_sharedmem_delete(lock->shared_mem);
	}
	else
	{
		if (lock->rwlock)
		{
			pthread_rwlock_destroy(lock->rwlock);
		}
	}

	cplus_free(lock);
	return CPLUS_SUCCESS;
}

static void * rwlock_initialize_object(CPLUS_RWLOCK_CONFIG config)
{
	bool has_attr = false, do_init = true;
	struct rwlock * lock = CPLUS_NULL;
	pthread_rwlockattr_t rwlock_attr;
	char rwlock_name[RWLOCK_NAME_MAX_SIZE + 1] = {0};
	struct named_rwlock * shared_rwlock;

	if (!(lock = (struct rwlock *)cplus_malloc(sizeof(struct rwlock))))
	{
		errno = ENOMEM;
		goto exit;
	}

	CPLUS_INITIALIZE_STRUCT_POINTER(lock);
	lock->type = OBJ_TYPE;
	lock->st_locked = false;
	lock->count_rdlocked = 0;
	lock->shared_mem = CPLUS_NULL;
	lock->rwlock = &(lock->nonamed_rwlock);

	if (CPLUS_RWLOCK_KIND_MAX > config->kind && CPLUS_RWLOCK_KIND_NONE != config->kind)
	{
		if (!has_attr && !pthread_rwlockattr_init(&(rwlock_attr)))
		{
			has_attr = true;
		}
		if (has_attr)
		{
			if (pthread_rwlockattr_setkind_np(&(rwlock_attr), convert_rwlock_kind(config->kind)))
			{
				goto exit;
			}
		}
	}

	if (CPLUS_NULL != config->name && CPLUS_RWLOCK_MODE_NONE != config->mode)
	{
		if (0 > cplus_str_printf(rwlock_name, RWLOCK_NAME_MAX_SIZE, RWLOCK_NAME_PATTERN, config->name))
		{
			goto exit;
		}

		switch(config->mode)
		{
		default:
			errno = EINVAL;
			goto exit;
		case CPLUS_RWLOCK_MODE_CREATE:
			{
				lock->shared_mem = cplus_sharedmem_create(rwlock_name
					, sizeof(struct named_rwlock));
			}
			break;
		case CPLUS_RWLOCK_MODE_OPEN:
			{
				lock->shared_mem = cplus_sharedmem_open(rwlock_name);
			}
			break;
		case CPLUS_RWLOCK_MODE_HYBRID:
			{
				lock->shared_mem = cplus_sharedmem_new(rwlock_name, sizeof(struct named_rwlock));
			}
			break;
		}
		if (!(lock->shared_mem))
		{
			goto exit;
		}

		if (!(shared_rwlock = (struct named_rwlock *)cplus_sharedmem_alloc(lock->shared_mem)))
		{
			goto exit;
		}
		lock->rwlock = &(shared_rwlock->rwlock);

		if (!cplus_sharedmem_is_owner(lock->shared_mem))
		{
			do_init = false;
		}
		else
		{
			if (!has_attr && !pthread_rwlockattr_init(&(rwlock_attr)))
			{
				has_attr = true;
			}
			if (has_attr)
			{
				if (pthread_rwlockattr_setpshared(&(rwlock_attr), PTHREAD_PROCESS_SHARED))
				{
					goto exit;
				}
			}
		}
	}

	if (do_init)
	{
		if(pthread_rwlock_init(lock->rwlock, (has_attr)? &(rwlock_attr): CPLUS_NULL))
		{
			goto exit;
		}
	}
	if (has_attr)
	{
		pthread_rwlockattr_destroy(&(rwlock_attr));
	}
	return lock;
exit:
	if (has_attr)
	{
		pthread_rwlockattr_destroy(&(rwlock_attr));
	}
	cplus_rwlock_delete(lock);
	return CPLUS_NULL;
}

cplus_rwlock cplus_rwlock_new_xp(const char * name)
{
	struct cplus_rwlock_config config;
	CHECK_NOT_NULL(name, CPLUS_NULL);
	CHECK_IF(RWLOCK_NAME_MAX_SIZE < (strlen(name) + RWLOCK_NAME_PATTERN_SIZE), CPLUS_NULL);

	CPLUS_MEM_CLEAN(&(config));
	config.name = (char *)(name);
	config.mode = CPLUS_RWLOCK_MODE_HYBRID;
	return rwlock_initialize_object(&(config));
}

cplus_rwlock cplus_rwlock_create_xp(const char * name)
{
	struct cplus_rwlock_config config;
	CHECK_NOT_NULL(name, CPLUS_NULL);
	CHECK_IF(RWLOCK_NAME_MAX_SIZE < (strlen(name) + RWLOCK_NAME_PATTERN_SIZE), CPLUS_NULL);

	CPLUS_MEM_CLEAN(&(config));
	config.name = (char *)(name);
	config.mode = CPLUS_RWLOCK_MODE_CREATE;
	return rwlock_initialize_object(&(config));
}

cplus_rwlock cplus_rwlock_open_xp(const char * name)
{
	struct cplus_rwlock_config config;
	CHECK_NOT_NULL(name, CPLUS_NULL);
	CHECK_IF(RWLOCK_NAME_MAX_SIZE < (strlen(name) + RWLOCK_NAME_PATTERN_SIZE), CPLUS_NULL);

	CPLUS_MEM_CLEAN(&(config));
	config.name = (char *)(name);
	config.mode = CPLUS_RWLOCK_MODE_OPEN;
	return rwlock_initialize_object(&(config));
}

cplus_rwlock cplus_rwlock_new_config(CPLUS_RWLOCK_CONFIG config)
{
	return rwlock_initialize_object(config);
}

cplus_rwlock cplus_rwlock_new(void)
{
	struct cplus_rwlock_config config;

	CPLUS_MEM_CLEAN(&(config));
	return rwlock_initialize_object(&(config));
}

bool cplus_rwlock_check(cplus_object obj)
{
	return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#ifdef __CPLUS_UNITTEST__
static char TEST_RWLOCK_NAME[] = "test";

CPLUS_UNIT_TEST(cplus_rwlock_new, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new())));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_new_xp, cross_process_test)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_wrlock, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new())));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 0));
	UNITTEST_EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 1500));
	UNITTEST_EXPECT_EQ(EDEADLK, errno);
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1500);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_wrlock, cross_process_test)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));

	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 0));
		UNITTEST_EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));

	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_ch, 0));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		UNITTEST_EXPECT_EQ(EDEADLK, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_rdlock, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new())));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 0));
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1500);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_sv, 0));
	UNITTEST_EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_sv, 1500));
	UNITTEST_EXPECT_EQ(EDEADLK, errno);
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1500);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_rdlock, cross_process_test)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 0));
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_sv, 1500));
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1500);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));


	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_ch, 0));
		UNITTEST_EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_rdlock(lock_ch, 1500));
		UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));

	if (0 != fork())
	{

	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_ch = cplus_rwlock_new_xp(TEST_RWLOCK_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_ch, 0));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_rdlock(lock_ch, 1500));
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_new_xp, bad_parameter)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	char bad_name[26] = "0123456789abcdef012345678";

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL == (lock_sv = cplus_rwlock_new_xp(bad_name))));
	UNITTEST_EXPECT_EQ(EINVAL, errno);
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_xp(&bad_name[1]))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_create_xp, functionity)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;

	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_create_xp(TEST_RWLOCK_NAME))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));

	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_ch = cplus_rwlock_open_xp(TEST_RWLOCK_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 0));
		UNITTEST_EXPECT_EQ(EBUSY, errno);
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));

	if (0 != fork())
	{
	}
	else
	{
		cplus_rwlock lock_ch = CPLUS_NULL;

		UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_ch = cplus_rwlock_create_xp(TEST_RWLOCK_NAME))));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_ch, 0));
		time = cplus_systime_get_tick();
		UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_ch, 1500));
		UNITTEST_EXPECT_EQ(EDEADLK, errno);
		time = cplus_systime_elapsed_tick(time);
		UNITTEST_EXPECT_EQ(true, time >= 1500 AND time < 1550);
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_ch));
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
		UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
		return;
	}

	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_rwlock_wrlock, PREFER_WRITER)
{
	cplus_rwlock lock_sv = CPLUS_NULL;
	uint32_t time;
	struct cplus_rwlock_config config = {0};

	config.kind = CPLUS_RWLOCK_KIND_PREFER_WRITER_NONRECURSIVE;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (lock_sv = cplus_rwlock_new_config(&config))));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_wrlock(lock_sv, CPLUS_INFINITE_TIMEOUT));
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 0));
	UNITTEST_EXPECT_EQ(EBUSY, errno);
	time = cplus_systime_get_tick();
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_rwlock_wrlock(lock_sv, 1500));
	UNITTEST_EXPECT_EQ(EDEADLK, errno);
	time = cplus_systime_elapsed_tick(time);
	UNITTEST_EXPECT_EQ(true, time < 1500);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_unlock(lock_sv));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_rwlock_delete(lock_sv));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_rwlock(void)
{
	UNITTEST_ADD_TESTCASE(cplus_rwlock_new, functionity);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_new_xp, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_wrlock, functionity);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_wrlock, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_rdlock, functionity);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_rdlock, cross_process_test);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_new_xp, bad_parameter);
	UNITTEST_ADD_TESTCASE(cplus_rwlock_wrlock, PREFER_WRITER);
}

#endif // __CPLUS_UNITTEST__