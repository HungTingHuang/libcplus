#include "internal.h"
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
		pthread_create(&tasks[inx], CPLUS_NULL, test_func, CPLUS_NULL);
	}
	for (inx = 0; inx < TASK_COUNT; inx++)
	{
		pthread_join(tasks[inx], CPLUS_NULL);
	}
}

void * task_atomic_add(void * args)
{
	for (int32_t i = 0; i < CALC_TIMES; i++)
	{
		cplus_atomic_add(&test_value, 1);
		// test_value++;
	}
	return CPLUS_NULL;
}

void * task_atomic_fetch_add(void * args)
{
	for (int32_t i = 0; i < CALC_TIMES; i++)
	{
		cplus_atomic_fetch_add(&test_value, 1);
		// test_value++;
	}
	return CPLUS_NULL;
}

TEST(cplus_atomic_read, functionity)
{
	test_value = 0x12345678;
	EXPECT_EQ(0x12345678, cplus_atomic_read(&test_value));
}

TEST(cplus_atomic_write, functionity)
{
	test_value = 0x12345678;
	EXPECT_EQ(0x87654321, (cplus_atomic_write(&test_value, 0x87654321), test_value));
}

TEST(cplus_atomic_exchange, functionity)
{
	test_value = 0x12345678;
	EXPECT_EQ(0x12345678, cplus_atomic_exchange(&test_value, 0x87654321));
	EXPECT_EQ(0x87654321, test_value);
}

TEST(cplus_atomic_add, functionity)
{
	test_value = 0;
	EXPECT_EQ(1, cplus_atomic_add(&test_value, 1));
	EXPECT_EQ(1, test_value);
}

TEST(cplus_atomic_add, counter)
{
	TASK_RUNNING(task_atomic_add);
	EXPECT_EQ(1000000000, test_value);
}

TEST(cplus_atomic_fetch_add, functionity)
{
	test_value = 0;
	EXPECT_EQ(CPLUS_SUCCESS, cplus_atomic_fetch_add(&test_value, 1));
	EXPECT_EQ(1, test_value);
}

TEST(cplus_atomic_fetch_add, counter)
{
	TASK_RUNNING(task_atomic_fetch_add);
	EXPECT_EQ(1000000000, test_value);
}

TEST(cplus_atomic_and, functionity)
{
	test_value = 0x0f0f0f0f;
	EXPECT_EQ(0x0f0f0f0f, cplus_atomic_and(&test_value, 0xffffffff));
	EXPECT_EQ(0x0f0f0f0f, test_value);
}

TEST(cplus_atomic_fetch_and, functionity)
{
	test_value = 0x0f0f0f0f;
	EXPECT_EQ(0x0f0f0f0f, cplus_atomic_fetch_and(&test_value, 0xf0f0f0f0));
	EXPECT_EQ(0x00000000, test_value);
}

TEST(cplus_atomic_or, functionity)
{
	test_value = 0x0f0f0f0f;
	EXPECT_EQ(0xffffffff, cplus_atomic_or(&test_value, 0xf0f0f0f0));
	EXPECT_EQ(0xffffffff, test_value);
}

TEST(cplus_atomic_fetch_or, functionity)
{
	test_value = 0x0000ffff;
	EXPECT_EQ(0x0000ffff, cplus_atomic_fetch_or(&test_value, 0xffff0000));
	EXPECT_EQ(0xffffffff, test_value);
}

TEST(cplus_atomic_xor, functionity)
{
	test_value = 0x0f0f0f0f;
	EXPECT_EQ(0xffffffff, cplus_atomic_xor(&test_value, 0xf0f0f0f0));
	EXPECT_EQ(0xffffffff, test_value);
}

TEST(cplus_atomic_fetch_xor, functionity)
{
	test_value = 0xff00ff00;
	EXPECT_EQ(0xff00ff00, cplus_atomic_fetch_xor(&test_value, 0xff00ff00));
	EXPECT_EQ(0x00000000, test_value);
}

TEST(cplus_atomic_nand, functionity)
{
	test_value = 0xffffffff;
	EXPECT_EQ(0x00000000, cplus_atomic_nand(&test_value, 0xffffffff));
	EXPECT_EQ(0x00000000, test_value);
}

TEST(cplus_atomic_fetch_nand, functionity)
{
	test_value = 0xffffffff;
	EXPECT_EQ(0xffffffff, cplus_atomic_fetch_nand(&test_value, 0xffffffff));
	EXPECT_EQ(0x00000000, test_value);
}

TEST(cplus_atomic_read_ptr, functionity)
{
	EXPECT_EQ(true, (test_value_ptr == cplus_atomic_read_ptr((void**)test_value_ptr_ptr)));
}

TEST(cplus_atomic_write_ptr, functionity)
{
	EXPECT_EQ(x, (cplus_atomic_write_ptr(&zp, xp), **((int32_t **)zpp)));
	EXPECT_EQ(y, (cplus_atomic_write_ptr(&xp, yp), **((int32_t **)xpp)));
	EXPECT_EQ(x, (cplus_atomic_write_ptr(&xp, zp), **((int32_t **)xpp)));
}

TEST(cplus_atomic_exchange_ptr, functionity)
{
	EXPECT_EQ(true, (*zpp == cplus_atomic_exchange_ptr(&zp, xp)));
	EXPECT_EQ(x, **((int32_t **)xpp));
	EXPECT_EQ(true, (*xpp == cplus_atomic_exchange_ptr(&xp, yp)));
	EXPECT_EQ(y, **((int32_t **)xpp));
	EXPECT_EQ(true, (*xpp == cplus_atomic_exchange_ptr(&xp, zp)));
	EXPECT_EQ(x, **((int32_t **)xpp));
	EXPECT_EQ(y, **((int32_t **)ypp));
	EXPECT_NE(z, **((int32_t **)zpp));
}

TEST(cplus_atomic_compare_exchange, functionity)
{
	int32_t currect_value = 0, expect_value = 0, new_value = 2;
	EXPECT_EQ(1, cplus_atomic_compare_exchange(&currect_value, &expect_value, &new_value));
	EXPECT_EQ(2, currect_value);
	EXPECT_EQ(3, (currect_value = 3));
	EXPECT_EQ(4, (expect_value = 4));
	EXPECT_EQ(0, cplus_atomic_compare_exchange(&currect_value, &expect_value, &new_value));
	EXPECT_EQ(3, currect_value);
}