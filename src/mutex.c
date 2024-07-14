/******************************************************************
* @file: mutex.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <pthread.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_mutex.h"
#include "cplus_sharedmem.h"
#include "cplus_systime.h"

#define OBJ_TYPE (OBJ_NONE + CTRL + 3)
#define MUTEX_NAME_MAX_SIZE 31
#define MUTEX_NAME_PATTERN "mutex.%s"
#define MUTEX_NAME_PATTERN_SIZE (sizeof(MUTEX_NAME_PATTERN) - 3)

struct named_mutex
{
    pthread_mutex_t mutex;
};

struct mutex
{
    uint16_t type;
    pthread_mutex_t * mutex;
    cplus_sharedmem shared_mem;
    pthread_mutex_t nonamed_mutex;
};

int32_t cplus_crit_sect_enter(cplus_mutex obj)
{
    if (0 != (errno = pthread_mutex_lock(((struct mutex *)obj)->mutex)))
    {
        return CPLUS_FAIL;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_crit_sect_exit(cplus_mutex obj)
{
    pthread_mutex_unlock(((struct mutex *)obj)->mutex);
    return CPLUS_SUCCESS;
}

int32_t cplus_mutex_lock(cplus_mutex obj, uint32_t timeout)
{
    int32_t res = CPLUS_SUCCESS;
    struct mutex * mtx = (struct mutex *)obj;

    CHECK_OBJECT_TYPE(obj);

    if (0 == timeout)
    {
        res = pthread_mutex_trylock(mtx->mutex);
    }
    else if (CPLUS_INFINITE_TIMEOUT == timeout)
    {
        res = pthread_mutex_lock(mtx->mutex);
    }
    else
    {
        struct timespec ts;
        cplus_systime_get_abstime_after_msec(&ts, timeout);
        res = pthread_mutex_timedlock(mtx->mutex, &ts);
    }

    if (0 != res)
    {
        errno = res;
        res = CPLUS_FAIL;
    }

    return res;
}

int32_t cplus_mutex_unlock(cplus_mutex obj)
{
    struct mutex * mtx = (struct mutex *)obj;

    CHECK_OBJECT_TYPE(obj);

    return pthread_mutex_unlock(mtx->mutex);
}

int32_t cplus_mutex_delete(cplus_mutex obj)
{
    struct mutex * mtx = (struct mutex *)obj;

    CHECK_OBJECT_TYPE(obj);

    if (mtx->shared_mem)
    {
        if (true == cplus_sharedmem_is_owner(mtx->shared_mem))
        {
            if (mtx->mutex)
            {
                pthread_mutex_destroy(mtx->mutex);
            }
        }
        cplus_sharedmem_delete(mtx->shared_mem);
    }
    else
    {
        if (mtx->mutex)
        {
            pthread_mutex_destroy(mtx->mutex);
        }
    }

    cplus_free(mtx);
    return CPLUS_SUCCESS;
}

static void * mutex_initialize_object(const char * name, enum INIT_MODE mode)
{
    struct mutex * mtx = NULL;

    if ((mtx = (struct mutex *)cplus_malloc(sizeof(struct mutex))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(mtx);

        mtx->type = OBJ_TYPE;
        mtx->mutex = NULL;
        mtx->shared_mem = NULL;

        if (NULL == name)
        {
            if (0 != pthread_mutex_init(&(mtx->nonamed_mutex), NULL))
            {
                goto exit;
            }
            mtx->mutex = &(mtx->nonamed_mutex);
        }
        else
        {
            char mutex_name[MUTEX_NAME_MAX_SIZE + 1] = {0};

            if (0 > cplus_str_printf(
                mutex_name
                , MUTEX_NAME_MAX_SIZE
                , MUTEX_NAME_PATTERN
                , name))
            {
                goto exit;
            }

            switch(mode)
            {
            case INIT_CREATE:
                mtx->shared_mem = cplus_sharedmem_create(
                    mutex_name
                    , sizeof(struct named_mutex));
                break;
            case INIT_OPEN:
                mtx->shared_mem = cplus_sharedmem_open(mutex_name);
                break;
            case INIT_HYBRID:
                mtx->shared_mem = cplus_sharedmem_new(
                    mutex_name
                    , sizeof(struct named_mutex));
                break;
            case INIT_NONE:
            default:
                errno = EINVAL;
                goto exit;
            }

            if (NULL == mtx->shared_mem)
            {
                goto exit;
            }
            else
            {
                struct named_mutex * shared_mutex = (struct named_mutex *)cplus_sharedmem_alloc(mtx->shared_mem);

                if (true == cplus_sharedmem_is_owner(mtx->shared_mem))
                {
                    pthread_mutexattr_t mutex_attr;

                    if (0 != pthread_mutexattr_init(&mutex_attr))
                    {
                        goto exit;
                    }

                    if (0 != pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED)
                        or 0 != pthread_mutex_init(&(shared_mutex->mutex), &mutex_attr))
                    {
                        pthread_mutexattr_destroy(&mutex_attr);
                        goto exit;
                    }
                }
                mtx->mutex = &(shared_mutex->mutex);
            }
        }
    }
    return mtx;
exit:
    cplus_mutex_delete(mtx);
    return NULL;
}

cplus_mutex cplus_mutex_new(void)
{
    return mutex_initialize_object(NULL, INIT_NONE);
}

cplus_mutex cplus_mutex_new_xp(const char * name)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_IF(MUTEX_NAME_MAX_SIZE < (strlen(name) + MUTEX_NAME_PATTERN_SIZE), NULL);

    return mutex_initialize_object(name, INIT_HYBRID);
}

cplus_mutex cplus_mutex_create_xp(const char * name)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_IF(MUTEX_NAME_MAX_SIZE < (strlen(name) + MUTEX_NAME_PATTERN_SIZE), NULL);

    return mutex_initialize_object(name, INIT_CREATE);
}

cplus_mutex cplus_mutex_open_xp(const char * name)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_IF(MUTEX_NAME_MAX_SIZE < (strlen(name) + MUTEX_NAME_PATTERN_SIZE), NULL);

    return mutex_initialize_object(name, INIT_OPEN);
}

bool cplus_mutex_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#ifdef __CPLUS_UNITTEST__
static char TEST_MUTEX_NAME[] = "test";

CPLUS_UNIT_TEST(cplus_mutex_new, functionity)
{
    cplus_mutex mtx = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mtx = cplus_mutex_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mutex_lock, functionity)
{
    cplus_mutex mtx = NULL;
    uint32_t time;

    UNITTEST_EXPECT_EQ(true, (NULL != (mtx = cplus_mutex_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_lock(mtx, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx, 0));
    UNITTEST_EXPECT_EQ(EBUSY, errno);
    time = cplus_systime_get_tick();
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx, 1500));
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(true, time >= 1500 and time < 1550);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_unlock(mtx));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}


CPLUS_UNIT_TEST(cplus_mutex_new_xp, cross_process_test)
{
    cplus_mutex mtx_sv = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mtx_sv = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_lock(mtx_sv, CPLUS_INFINITE_TIMEOUT));

    if (0 != fork())
    {
    }
    else
    {
        cplus_mutex mtx_ch = NULL;
        uint32_t time;

        UNITTEST_EXPECT_EQ(true, (NULL != (mtx_ch = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
        UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 0));
        UNITTEST_EXPECT_EQ(EBUSY, errno);
        time = cplus_systime_get_tick();
        UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 1500));
        UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
        time = cplus_systime_elapsed_tick(time);
        UNITTEST_EXPECT_EQ(true, time >= 1500 and time < 1550);

        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
        UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
        return;
    }

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mutex_create_xp, cross_process_test)
{
    cplus_mutex mtx_sv = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mtx_sv = cplus_mutex_create_xp(TEST_MUTEX_NAME))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_lock(mtx_sv, CPLUS_INFINITE_TIMEOUT));

    if (0 != fork())
    {
    }
    else
    {
        cplus_mutex mtx_ch = NULL;
        uint32_t time;

        UNITTEST_EXPECT_EQ(true, (NULL != (mtx_ch = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
        UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 0));
        UNITTEST_EXPECT_EQ(EBUSY, errno);
        time = cplus_systime_get_tick();
        UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mutex_lock(mtx_ch, 1500));
        UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
        time = cplus_systime_elapsed_tick(time);
        UNITTEST_EXPECT_EQ(true, time >= 1500 and time < 1550);

        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
        UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
        return;
    }

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mutex_new_xp, bad_parameter)
{
    cplus_mutex mtx = NULL;
    char bad_name[27] = "0123456789abcdef0123456789";

    UNITTEST_EXPECT_EQ(true, (NULL == (mtx = cplus_mutex_new_xp(bad_name))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL != (mtx = cplus_mutex_new_xp(&bad_name[1]))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mutex_create_xp, bad_parameter)
{
    cplus_mutex mtx = NULL;
    char bad_name[27] = "0123456789abcdef0123456789";

    UNITTEST_EXPECT_EQ(true, (NULL == (mtx = cplus_mutex_create_xp(bad_name))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL != (mtx = cplus_mutex_create_xp(&bad_name[1]))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mutex_open_xp, bad_parameter)
{
    cplus_mutex mtx = NULL;
    char bad_name[27] = "0123456789abcdef0123456789";

    UNITTEST_EXPECT_EQ(true, (NULL == (mtx = cplus_mutex_open_xp(bad_name))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (mtx = cplus_mutex_open_xp(&bad_name[1]))));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_crit_sect_enter, functionity)
{
    cplus_mutex mtx = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mtx = cplus_mutex_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_crit_sect_enter, cross_process_test)
{
    cplus_mutex mtx_sv = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mtx_sv = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx_sv));

    if (0 != fork())
    {
    }
    else
    {
        cplus_mutex mtx_ch = NULL;

        UNITTEST_EXPECT_EQ(true, (NULL != (mtx_ch = cplus_mutex_new_xp(TEST_MUTEX_NAME))));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_enter(mtx_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_crit_sect_exit(mtx_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
        UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
        return;
    }

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mutex_delete(mtx_sv));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_mutex(void)
{
    UNITTEST_ADD_TESTCASE(cplus_mutex_new, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mutex_lock, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mutex_new_xp, cross_process_test);
    UNITTEST_ADD_TESTCASE(cplus_mutex_create_xp, cross_process_test);
    UNITTEST_ADD_TESTCASE(cplus_mutex_new_xp, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_mutex_create_xp, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_mutex_open_xp, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_crit_sect_enter, functionity);
    UNITTEST_ADD_TESTCASE(cplus_crit_sect_enter, cross_process_test);
}
#endif // __CPLUS_UNITTEST__

