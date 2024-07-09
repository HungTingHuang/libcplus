/******************************************************************
* @file: atomic.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
*
* @details:
*   Built-in Functions for Memory Model Aware Atomic Operations
*   ref: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
*
*   __sync_* family of functions:
*   ref: https://gcc.gnu.org/onlinedocs/gcc-4.6.3/gcc/Atomic-Builtins.html
*
******************************************************************/

#include "common.h"
#include "cplus_atomic.h"

void * cplus_atomic_read_ptr(void ** ptr)
{
    #if (40700 <= GCC_VERSION)
    {
        return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
    }
    #elif (40603 <= GCC_VERSION)
    {
        errno = ENOTSUP;
        return NULL;
    }
    #else
    {
        errno = ENOTSUP;
        return NULL;
    }
    #endif

}

void cplus_atomic_write_ptr(void ** ptr, void * value)
{
    #if (40700 <= GCC_VERSION)
    {
        __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
    }
    #elif (40603 <= GCC_VERSION)
    {
        errno = ENOTSUP;
    }
    #else
    {
        errno = ENOTSUP;
    }
    #endif
}

void * cplus_atomic_exchange_ptr(void ** ptr, void * value)
{
	#if (40700 <= GCC_VERSION)
    {
        return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
    }
    #elif (40603 <= GCC_VERSION)
    {
        errno = ENOTSUP;
        return NULL;
    }
    #else
    {
        errno = ENOTSUP;
        return NULL;
    }
    #endif
}

#ifdef __CPLUS_UNITTEST__
#include <pthread.h>

#define TASK_COUNT 100
#define CALC_TIMES 10000000

static int32_t test_value = 0, inx = 0;
static int32_t *test_value_ptr = &test_value;
static int32_t **test_value_ptr_ptr = &test_value_ptr;

static int32_t x = 0x12345678, y = 0x87654321, z = 0x00000000;
static void *xp = &x, *yp = &y, *zp = &z;
static void **xpp = &xp, **ypp = &yp, **zpp = &zp;

static pthread_t tasks[TASK_COUNT];

static void TASK_RUNNING(void * (*test_func)(void *))
{
    test_value = 0;
    for (inx = 0; inx < TASK_COUNT; inx++)
    {
        pthread_create(&tasks[inx], NULL, test_func, NULL);
    }
    for (inx = 0; inx < TASK_COUNT; inx++)
    {
        pthread_join(tasks[inx], NULL);
    }
}

void * task_atomic_add(void * args)
{
    for (int32_t i = 0; i < CALC_TIMES; i++)
    {
        cplus_atomic_add(&test_value, 1);
        // test_value++;
    }
    return NULL;
}

void * task_atomic_fetch_add(void * args)
{
    for (int32_t i = 0; i < CALC_TIMES; i++)
    {
        cplus_atomic_fetch_add(&test_value, 1);
        // test_value++;
    }
    return NULL;
}

CPLUS_UNIT_TEST(cplus_atomic_read, functionity)
{
    test_value = 0x12345678;
    UNITTEST_EXPECT_EQ(0x12345678, cplus_atomic_read(&test_value));
}

CPLUS_UNIT_TEST(cplus_atomic_write, functionity)
{
    test_value = 0x12345678;
    UNITTEST_EXPECT_EQ(0x87654321, (cplus_atomic_write(&test_value, 0x87654321), test_value));
}

CPLUS_UNIT_TEST(cplus_atomic_exchange, functionity)
{
    test_value = 0x12345678;
    UNITTEST_EXPECT_EQ(0x12345678, cplus_atomic_exchange(&test_value, 0x87654321));
    UNITTEST_EXPECT_EQ(0x87654321, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_add, functionity)
{
    test_value = 0;
    UNITTEST_EXPECT_EQ(1, cplus_atomic_add(&test_value, 1));
    UNITTEST_EXPECT_EQ(1, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_add, counter)
{
    TASK_RUNNING(task_atomic_add);
    UNITTEST_EXPECT_EQ(1000000000, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_fetch_add, functionity)
{
    test_value = 0;
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_atomic_fetch_add(&test_value, 1));
    UNITTEST_EXPECT_EQ(1, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_fetch_add, counter)
{
    TASK_RUNNING(task_atomic_fetch_add);
    UNITTEST_EXPECT_EQ(1000000000, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_and, functionity)
{
    test_value = 0x0f0f0f0f;
    UNITTEST_EXPECT_EQ(0x0f0f0f0f, cplus_atomic_and(&test_value, 0xffffffff));
    UNITTEST_EXPECT_EQ(0x0f0f0f0f, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_fetch_and, functionity)
{
    test_value = 0x0f0f0f0f;
    UNITTEST_EXPECT_EQ(0x0f0f0f0f, cplus_atomic_fetch_and(&test_value, 0xf0f0f0f0));
    UNITTEST_EXPECT_EQ(0x00000000, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_or, functionity)
{
    test_value = 0x0f0f0f0f;
    UNITTEST_EXPECT_EQ(0xffffffff, cplus_atomic_or(&test_value, 0xf0f0f0f0));
    UNITTEST_EXPECT_EQ(0xffffffff, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_fetch_or, functionity)
{
    test_value = 0x0000ffff;
    UNITTEST_EXPECT_EQ(0x0000ffff, cplus_atomic_fetch_or(&test_value, 0xffff0000));
    UNITTEST_EXPECT_EQ(0xffffffff, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_xor, functionity)
{
    test_value = 0x0f0f0f0f;
    UNITTEST_EXPECT_EQ(0xffffffff, cplus_atomic_xor(&test_value, 0xf0f0f0f0));
    UNITTEST_EXPECT_EQ(0xffffffff, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_fetch_xor, functionity)
{
    test_value = 0xff00ff00;
    UNITTEST_EXPECT_EQ(0xff00ff00, cplus_atomic_fetch_xor(&test_value, 0xff00ff00));
    UNITTEST_EXPECT_EQ(0x00000000, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_nand, functionity)
{
    test_value = 0xffffffff;
    UNITTEST_EXPECT_EQ(0x00000000, cplus_atomic_nand(&test_value, 0xffffffff));
    UNITTEST_EXPECT_EQ(0x00000000, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_fetch_nand, functionity)
{
    test_value = 0xffffffff;
    UNITTEST_EXPECT_EQ(0xffffffff, cplus_atomic_fetch_nand(&test_value, 0xffffffff));
    UNITTEST_EXPECT_EQ(0x00000000, test_value);
}

CPLUS_UNIT_TEST(cplus_atomic_read_ptr, functionity)
{
    UNITTEST_EXPECT_EQ(true, (test_value_ptr == cplus_atomic_read_ptr((void**)test_value_ptr_ptr)));
}

CPLUS_UNIT_TEST(cplus_atomic_write_ptr, functionity)
{
    UNITTEST_EXPECT_EQ(x, (cplus_atomic_write_ptr(&zp, xp), **((int32_t **)zpp)));
    UNITTEST_EXPECT_EQ(y, (cplus_atomic_write_ptr(&xp, yp), **((int32_t **)xpp)));
    UNITTEST_EXPECT_EQ(x, (cplus_atomic_write_ptr(&xp, zp), **((int32_t **)xpp)));
}

CPLUS_UNIT_TEST(cplus_atomic_exchange_ptr, functionity)
{
    UNITTEST_EXPECT_EQ(true, (*zpp == cplus_atomic_exchange_ptr(&zp, xp)));
    UNITTEST_EXPECT_EQ(x, **((int32_t **)xpp));
    UNITTEST_EXPECT_EQ(true, (*xpp == cplus_atomic_exchange_ptr(&xp, yp)));
    UNITTEST_EXPECT_EQ(y, **((int32_t **)xpp));
    UNITTEST_EXPECT_EQ(true, (*xpp == cplus_atomic_exchange_ptr(&xp, zp)));
    UNITTEST_EXPECT_EQ(x, **((int32_t **)xpp));
    UNITTEST_EXPECT_EQ(y, **((int32_t **)ypp));
    UNITTEST_EXPECT_NE(z, **((int32_t **)zpp));
}

CPLUS_UNIT_TEST(cplus_atomic_compare_exchange, functionity)
{
    int32_t currect_value = 0, expect_value = 0, new_value = 2;

    UNITTEST_EXPECT_EQ(1, cplus_atomic_compare_exchange(&currect_value, &expect_value, &new_value));
    UNITTEST_EXPECT_EQ(2, currect_value);
    UNITTEST_EXPECT_EQ(3, (currect_value = 3));
    UNITTEST_EXPECT_EQ(4, (expect_value = 4));
    UNITTEST_EXPECT_EQ(0, cplus_atomic_compare_exchange(&currect_value, &expect_value, &new_value));
    UNITTEST_EXPECT_EQ(3, currect_value);
}

void unittest_atomic(void)
{
    UNITTEST_ADD_TESTCASE(cplus_atomic_read, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_write, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_exchange, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_add, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_add, counter);
    UNITTEST_ADD_TESTCASE(cplus_atomic_fetch_add, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_fetch_add, counter);
    UNITTEST_ADD_TESTCASE(cplus_atomic_and, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_fetch_and, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_or, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_fetch_or, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_xor, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_fetch_xor, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_nand, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_fetch_nand, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_read_ptr, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_write_ptr, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_exchange_ptr, functionity);
    UNITTEST_ADD_TESTCASE(cplus_atomic_compare_exchange, functionity);
}

#endif // __UNITTEST__