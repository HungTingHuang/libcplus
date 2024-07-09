/******************************************************************
* @file: semaphore.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <linux/sem.h>
#include <semaphore.h>
#include <fcntl.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_systime.h"
#include "cplus_semaphore.h"

#define OBJ_TYPE (OBJ_NONE + CTRL + 2)
#define SEMAPHORE_MAX_VALUE SEMVMX
#define SEMAPHORE_NAME_SIZE 31
#define SEMAPHORE_NAME_PATTERN "cplus.sema_%s"
#define SEMAPHORE_NAME_PATTERN_SIZE (sizeof(SEMAPHORE_NAME_PATTERN) - 3)

struct semaphore
{
    uint16_t type;
    sem_t * psem;
    sem_t nonamed_sem;
    volatile bool is_named;
    volatile bool is_owner;
    char name[SEMAPHORE_NAME_SIZE + 1];
};

int32_t cplus_semaphore_get_value(cplus_semaphore obj)
{
    int32_t pc = 0;
    struct semaphore * sema = (struct semaphore *)obj;

    CHECK_OBJECT_TYPE(obj);

    if (0 == sem_getvalue(sema->psem, &pc))
    {
        return pc;
    }
    return CPLUS_FAIL;
}

int32_t cplus_semaphore_push(cplus_semaphore obj, int32_t count)
{
    struct semaphore * sema = (struct semaphore *)obj;
    int32_t remain = (SEMAPHORE_MAX_VALUE - cplus_semaphore_get_value(obj));

    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(SEMAPHORE_MAX_VALUE < remain, -1);
    CHECK_IN_INTERVAL(count, 1, remain, -1);

    while ((count --) > 0)
    {
        if (0 != sem_post(sema->psem))
        {
            return CPLUS_FAIL;
        }
    }

    return CPLUS_SUCCESS;
}

int32_t cplus_semaphore_wait_poll(cplus_semaphore obj, uint32_t timeout)
{
    int32_t res = CPLUS_SUCCESS;
    struct semaphore * sema = (struct semaphore *)obj;

    CHECK_OBJECT_TYPE(obj);

    if (0 == timeout)
    {
        res = sem_trywait(sema->psem);
    }
    else if (CPLUS_INFINITE_TIMEOUT == timeout)
    {
        res = sem_wait(sema->psem);
    }
    else
    {
        struct timespec ts;
        cplus_systime_get_abstime_after_msec(&ts, timeout);
        res = sem_timedwait(sema->psem, &ts);
    }
    return res;
}

int32_t cplus_semaphore_delete(cplus_semaphore obj)
{
    int32_t res = CPLUS_SUCCESS;
    struct semaphore * sema = (struct semaphore *)obj;

    CHECK_OBJECT_TYPE(obj);

    if ((sem_t *)SEM_FAILED != sema->psem)
    {
        if (sema->is_named)
        {
            sem_close(sema->psem);
            if (sema->is_owner)
            {
                sem_unlink(sema->name);
            }
        }
        else
        {
            sem_destroy(sema->psem);
        }
    }

    cplus_free(sema);
    return res;
}

static void * semaphore_xp_create(const char * name, int32_t initial_count)
{
    struct semaphore * sema = NULL;

    sema = (struct semaphore *)cplus_malloc(sizeof(struct semaphore));
    if (sema)
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(sema);

        sema->type = OBJ_TYPE;
        sema->psem = NULL;
        sema->is_named = false;
        sema->is_owner = false;

        if (0 > cplus_str_printf(
            sema->name
            , SEMAPHORE_NAME_SIZE
            , SEMAPHORE_NAME_PATTERN
            , name))
        {
            goto exit;
        }

        sema->psem = sem_open(
            sema->name
            , O_CREAT | O_EXCL
            , DEFFILEMODE
            , initial_count);

        if ((sem_t *)SEM_FAILED == sema->psem)
        {
            goto exit;
        }
        else
        {
            sema->is_owner = true;
            sema->is_named = true;
        }

        return sema;
    }
exit:
    cplus_semaphore_delete(sema);
    return NULL;
}

static void * semaphore_xp_open(const char * name)
{
    struct semaphore * sema = NULL;

    sema = (struct semaphore *)cplus_malloc(sizeof(struct semaphore));
    if (sema)
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(sema);

        sema->type = OBJ_TYPE;
        sema->psem = NULL;
        sema->is_named = false;
        sema->is_owner = false;

        if (0 > cplus_str_printf(
            sema->name
            , SEMAPHORE_NAME_SIZE
            , SEMAPHORE_NAME_PATTERN
            , name))
        {
            goto exit;
        }

        sema->psem = sem_open(sema->name, O_RDWR);
        if ((sem_t *)SEM_FAILED == sema->psem)
        {
            goto exit;
        }
        else
        {
            sema->is_named = true;
        }

        return sema;
    }
exit:
    cplus_semaphore_delete(sema);
    return NULL;
}

static void * semaphore_xp_new(const char * name, int32_t initial_count)
{
    struct semaphore * sema = NULL;

    sema = semaphore_xp_open(name);
    if (NULL == sema)
    {
        if (ENOENT == errno)
        {
            sema = semaphore_xp_create(name, initial_count);
        }
    }

    return sema;
}

cplus_semaphore cplus_semaphore_new(int32_t initial_count)
{
    struct semaphore * sema = NULL;
    CHECK_IN_INTERVAL(initial_count, 0, SEMAPHORE_MAX_VALUE, NULL);

    sema = (struct semaphore *)cplus_malloc(sizeof(struct semaphore));
    if (sema)
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(sema);

        sema->type = OBJ_TYPE;
        sema->psem = NULL;
        sema->is_named = false;
        sema->is_owner = false;

        if (0 != sem_init(&(sema->nonamed_sem), 0, initial_count))
        {
            goto exit;
        }
        sema->psem = &(sema->nonamed_sem);

        return sema;
    }
exit:
    cplus_semaphore_delete(sema);
    return NULL;
}

cplus_semaphore cplus_semaphore_new_xp(
    const char * name
    , int32_t initial_count)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_IF(
        SEMAPHORE_NAME_SIZE < (strlen(name) + SEMAPHORE_NAME_PATTERN_SIZE)
        , NULL);
    CHECK_IN_INTERVAL(initial_count, 0, SEMAPHORE_MAX_VALUE, NULL);
    return semaphore_xp_new(name, initial_count);
}

cplus_semaphore cplus_semaphore_create_xp(
    const char * name
    , int32_t initial_count)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_IF(
        SEMAPHORE_NAME_SIZE < (strlen(name) + SEMAPHORE_NAME_PATTERN_SIZE)
        , NULL);
    CHECK_IN_INTERVAL(
        initial_count
        , 0
        , SEMAPHORE_MAX_VALUE
        , NULL);
    return semaphore_xp_create(name, initial_count);
}

cplus_semaphore cplus_semaphore_open_xp(const char * name)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_IF(
        SEMAPHORE_NAME_SIZE < (strlen(name) + SEMAPHORE_NAME_PATTERN_SIZE)
        , NULL);
    return semaphore_xp_open(name);
}

bool cplus_semaphore_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#ifdef __CPLUS_UNITTEST__
static char TEST_SEMAPHORE_NAME[] = "test";

CPLUS_UNIT_TEST(cplus_semaphore_new, functionity)
{
    cplus_semaphore semp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (semp = cplus_semaphore_new(10))));
    UNITTEST_EXPECT_EQ(10, cplus_semaphore_get_value(semp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_push, functionity)
{
    cplus_semaphore semp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (semp = cplus_semaphore_new(10))));
    UNITTEST_EXPECT_EQ(10, cplus_semaphore_get_value(semp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp, 10));
    UNITTEST_EXPECT_EQ(20, cplus_semaphore_get_value(semp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp, 20));
    UNITTEST_EXPECT_EQ(40, cplus_semaphore_get_value(semp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_wait_poll, functionity)
{
    cplus_semaphore semp = NULL;
    uint32_t time;

    UNITTEST_EXPECT_EQ(true, (NULL != (semp = cplus_semaphore_new(10))));
    for (int32_t i = 10; i > 0; i--)
    {
        UNITTEST_EXPECT_EQ(i, cplus_semaphore_get_value(semp));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp, 0));
    }
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp, 0));
    UNITTEST_EXPECT_EQ(EAGAIN, errno);

    time = cplus_systime_get_tick();
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp, 1500));
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    time = cplus_systime_elapsed_tick(time);
    UNITTEST_EXPECT_EQ(true, time >= 1500 and time < 1550);

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_new, bad_parameter)
{
    cplus_semaphore semp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL == (semp = cplus_semaphore_new(SEMAPHORE_MAX_VALUE + 1))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp = cplus_semaphore_new(-111))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_push, bad_parameter)
{
    cplus_semaphore semp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (semp = cplus_semaphore_new(10))));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_push(semp, SEMAPHORE_MAX_VALUE - 9));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp, SEMAPHORE_MAX_VALUE - 10));
    UNITTEST_EXPECT_EQ(SEMAPHORE_MAX_VALUE, cplus_semaphore_get_value(semp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_wait_poll, bad_parameter)
{
    cplus_semaphore semp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (semp = cplus_semaphore_new(10))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_new_xp, cross_process_test)
{
    cplus_semaphore semp_sv = NULL, semp_ch = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (semp_sv = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, 10))));
    UNITTEST_EXPECT_EQ(10, cplus_semaphore_get_value(semp_sv));

    if (0 != fork())
    {
    }
    else
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (semp_ch = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, 1))));
        UNITTEST_EXPECT_EQ(10, cplus_semaphore_get_value(semp_ch));
        for (int32_t i = 10; i > 0; i--)
        {
            UNITTEST_EXPECT_EQ(i, cplus_semaphore_get_value(semp_ch));
            UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp_ch, 0));
        }
        UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp_ch, 0));
        UNITTEST_EXPECT_EQ(EAGAIN, errno);
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
        UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
        return;
    }

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_create_xp, cross_process_test)
{
    cplus_semaphore semp_sv = NULL, semp_ch = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (semp_sv = cplus_semaphore_create_xp(TEST_SEMAPHORE_NAME, 10))));
    UNITTEST_EXPECT_EQ(10, cplus_semaphore_get_value(semp_sv));

    for (int32_t i = 10; i > 0; i--)
    {
        UNITTEST_EXPECT_EQ(i, cplus_semaphore_get_value(semp_sv));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp_sv, 0));
    }
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_semaphore_wait_poll(semp_sv, 0));
    UNITTEST_EXPECT_EQ(EAGAIN, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_push(semp_sv, 1));

    if (0 != fork())
    {
    }
    else
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (semp_ch = cplus_semaphore_open_xp(TEST_SEMAPHORE_NAME))));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_wait_poll(semp_ch, CPLUS_INFINITE_TIMEOUT));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_ch));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
        UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
        return;
    }

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_new_xp, bad_parameter)
{
    cplus_semaphore semp_sv = NULL;
    char bad_name[23] = "0123456789abcdef012345";

    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_new_xp(NULL, 10))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_new_xp(bad_name, 10))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL != (semp_sv = cplus_semaphore_new_xp(&bad_name[1], 10))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, SEMAPHORE_MAX_VALUE + 1))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_new_xp(TEST_SEMAPHORE_NAME, -45465))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_create_xp, bad_parameter)
{
    cplus_semaphore semp_sv = NULL, semp_ch = NULL;
    char bad_name[23] = "0123456789abcdef012345";

    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_create_xp(NULL, 10))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_create_xp(bad_name, 10))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL != (semp_sv = cplus_semaphore_create_xp(&bad_name[3], 10))));
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_ch = cplus_semaphore_create_xp(&bad_name[3], 10))));
    UNITTEST_EXPECT_EQ(EEXIST, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_semaphore_delete(semp_sv));
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_create_xp(TEST_SEMAPHORE_NAME, SEMAPHORE_MAX_VALUE + 1))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_create_xp(TEST_SEMAPHORE_NAME, -45465))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);

    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_semaphore_open_xp, bad_parameter)
{
    cplus_semaphore semp_sv = NULL;
    char bad_name[23] = "0123456789abcdef012345";

    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_open_xp(NULL))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_open_xp(bad_name))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (semp_sv = cplus_semaphore_open_xp(&bad_name[3]))));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_semaphore(void)
{
    UNITTEST_ADD_TESTCASE(cplus_semaphore_new, functionity);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_push, functionity);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_wait_poll, functionity);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_new, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_push, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_wait_poll, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_new_xp, cross_process_test);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_create_xp, cross_process_test);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_new_xp, cross_process_test);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_create_xp, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_semaphore_open_xp, bad_parameter);
}

#endif // __CPLUS_UNITTEST__