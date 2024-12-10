#include "internal.h"
#define MAX_TEST_SIZE 1024
int32_t *a = CPLUS_NULL, *b = CPLUS_NULL, *c = CPLUS_NULL, *d = CPLUS_NULL, *e = CPLUS_NULL, *f = CPLUS_NULL;
int64_t *g = CPLUS_NULL, *h = CPLUS_NULL, *i = CPLUS_NULL, *j = CPLUS_NULL, *k = CPLUS_NULL, *l = CPLUS_NULL;
int32_t *ry[MAX_TEST_SIZE];
int64_t *ry2[MAX_TEST_SIZE];

static struct test1
{
	int32_t A;
	int32_t B;
	int32_t C;
	int32_t D;
	int32_t E;
} t1 = { .A = 0x0123, .B = 0x1234, .C = 0x2345, .D = 0x3456, .E = 0x4567 };

TEST(cplus_mgr_malloc, functionity)
{
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(a));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(b));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(c));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(d));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(e));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(f));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mgr_malloc, stress)
{
	for (int32_t i = 0; i < MAX_TEST_SIZE; i++)
	{
		EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (ry[i] = (int32_t *)cplus_malloc(sizeof(int32_t))));
	}

#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(MAX_TEST_SIZE, cplus_mgr_report());
#endif

	for (int32_t i = 0; i < MAX_TEST_SIZE; i++)
	{
		EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (ry2[i] = (int64_t *)cplus_realloc(ry[i], sizeof(int64_t))));
	}

#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(MAX_TEST_SIZE, cplus_mgr_report());
#endif

	for (int32_t i = 0; i< MAX_TEST_SIZE; i++)
	{
		EXPECT_EQ(CPLUS_SUCCESS, cplus_free(ry2[i]));
	}
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mgr_realloc, functionity)
{
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (g = (int64_t *)cplus_realloc(a, sizeof(int64_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (h = (int64_t *)cplus_realloc(b, sizeof(int64_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (i = (int64_t *)cplus_realloc(c, sizeof(int64_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (j = (int64_t *)cplus_realloc(d, sizeof(int64_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (k = (int64_t *)cplus_realloc(e, sizeof(int64_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (l = (int64_t *)cplus_realloc(f, sizeof(int64_t))));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(g));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(h));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(i));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(j));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(k));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(l));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mgr_realloc, bad_parameter)
{
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (g = (int64_t *)cplus_realloc(a, 0)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (h = (int64_t *)cplus_realloc(b, 0)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (i = (int64_t *)cplus_realloc(c, 0)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (j = (int64_t *)cplus_realloc(d, 0)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (k = (int64_t *)cplus_realloc(e, 0)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (l = (int64_t *)cplus_realloc(f, 0)));
	EXPECT_EQ(ENOMEM, errno);
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(0, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (g = (int64_t *)cplus_realloc(a
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (h = (int64_t *)cplus_realloc(b
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (i = (int64_t *)cplus_realloc(c
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (j = (int64_t *)cplus_realloc(d
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (k = (int64_t *)cplus_realloc(e
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (l = (int64_t *)cplus_realloc(f
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(CPLUS_MEM_CLEAN, functionity)
{
	EXPECT_EQ(0x0123, t1.A);
	EXPECT_EQ(0x1234, t1.B);
	EXPECT_EQ(0x2345, t1.C);
	EXPECT_EQ(0x3456, t1.D);
	EXPECT_EQ(0x4567, t1.E);
	CPLUS_MEM_CLEAN(&(t1));
	EXPECT_EQ(0, t1.A);
	EXPECT_EQ(0, t1.B);
	EXPECT_EQ(0, t1.C);
	EXPECT_EQ(0, t1.D);
	EXPECT_EQ(0, t1.E);
}

#ifdef __CPLUS_MEM_MANAGER__
TEST(cplus_mgr_malloc, buffer_overrun)
{
	uint8_t * str = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	EXPECT_EQ(1, cplus_mgr_report());
	str[10] = '\0';
	EXPECT_EQ(CPLUS_FAIL, cplus_mgr_check_boundary(str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mgr_realloc, buffer_overrun)
{
	uint8_t * str = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	EXPECT_EQ(1, cplus_mgr_report());
	str[9] = '\0';
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_realloc(str, 15)));
	EXPECT_EQ(1, cplus_mgr_report());
	str[15] = '\0';
	EXPECT_EQ(CPLUS_FAIL, cplus_mgr_check_boundary(str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mgr_malloc, double_free)
{
	uint8_t * str = CPLUS_NULL, * temp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	temp = str;
	EXPECT_EQ(1, cplus_mgr_report());
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(temp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mgr_realloc, double_free)
{
	uint8_t * str = CPLUS_NULL, * temp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	EXPECT_EQ(1, cplus_mgr_report());
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_realloc(str, 15)));
	EXPECT_EQ(1, cplus_mgr_report());
	temp = str;
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(temp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mem_cpy, overflow)
{
	uint8_t * str = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	EXPECT_EQ(CPLUS_TRUE, (str == cplus_mem_cpy(str, (void *)("012345"), strlen("012345"))));
}

TEST(cplus_mem_cpy_ex, overflow)
{
	uint8_t * str = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	EXPECT_EQ(CPLUS_TRUE, (str == cplus_mem_cpy_ex(str, strlen("012345"), (void *)("012345"), strlen("012345"))));
}

TEST(cplus_mem_set, overflow)
{
	uint8_t * str = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	EXPECT_EQ(CPLUS_TRUE, (str == cplus_mem_set(str, 0x00, 6)));
}

TEST(cplus_str_printf, overflow)
{
	uint8_t * str = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	EXPECT_EQ(CPLUS_TRUE, (strlen("012345") == cplus_str_printf(str, strlen("012345"), "%s", strlen("012345"))));
}
#endif