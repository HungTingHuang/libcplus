#include "internal.h"

static int32_t num0 = 0, num1 = 1, num2 = 2, num3 = 3, num4 = 4, num5 = 5;

static int32_t find_num(void * data, void * arg)
{
	if (*((int32_t *)data) == *((int32_t *)arg))
	{
		return 0;
	}
	return 1;
}

static int32_t compare_num(void * data1, void * data2)
{
	if (*((int32_t *)data1) == *((int32_t *)data2))
	{
		return 0;
	}
	return 1;
}

TEST(cplus_llist_push_front, sequence)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_front, bad_parameter)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_back(list, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_back, sequence)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 2, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_back, bad_parameter)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_back(list, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_at, push_randomly)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 2, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_pop_of, pop_randomly)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_of(list, 0)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_of(list, 3)));
	EXPECT_EQ(2, cplus_llist_get_size(list));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_index_if, functionity)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(0, cplus_llist_get_index_if(list, find_num, &num0));
	EXPECT_EQ(1, cplus_llist_get_index_if(list, find_num, &num1));
	EXPECT_EQ(2, cplus_llist_get_index_if(list, find_num, &num2));
	EXPECT_EQ(3, cplus_llist_get_index_if(list, find_num, &num3));
	EXPECT_EQ(4, cplus_llist_get_index_if(list, find_num, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_index_if, bad_parameter)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, CPLUS_NULL, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, find_num, &num5));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_front, sequence_with_mempool)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_prev_new(1024))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
}

TEST(cplus_llist_push_front, bad_parameter_with_mempool)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_prev_new(1024))));
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(CPLUS_LLIST_FOREACH, sequence)
{
	cplus_llist list = CPLUS_NULL;
	int * data = CPLUS_NULL, value = 4;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	CPLUS_LLIST_FOREACH(list, data)
	{
		EXPECT_EQ(value, *data);
		value --;
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_set_if, functionity)
{
	cplus_llist list = CPLUS_NULL;
	cplus_llist list_0 = CPLUS_NULL, list_1 = CPLUS_NULL, list_2 = CPLUS_NULL;
	cplus_llist list_3 = CPLUS_NULL, list_4 = CPLUS_NULL, list_5 = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(15, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(16, cplus_mgr_report());
#endif
	EXPECT_NE(CPLUS_TRUE, (CPLUS_NULL != (list_0 = cplus_llist_get_set_if(list, find_num, &num0))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_1 = cplus_llist_get_set_if(list, find_num, &num1))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_2 = cplus_llist_get_set_if(list, find_num, &num2))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_3 = cplus_llist_get_set_if(list, find_num, &num3))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_4 = cplus_llist_get_set_if(list, find_num, &num4))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_5 = cplus_llist_get_set_if(list, find_num, &num5))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(1, cplus_llist_get_size(list_1));
	EXPECT_EQ(2, cplus_llist_get_size(list_2));
	EXPECT_EQ(3, cplus_llist_get_size(list_3));
	EXPECT_EQ(4, cplus_llist_get_size(list_4));
	EXPECT_EQ(5, cplus_llist_get_size(list_5));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list_1)));
	EXPECT_EQ(0, cplus_llist_get_size(list_1));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
	EXPECT_EQ(1, cplus_llist_get_size(list_2));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
	EXPECT_EQ(0, cplus_llist_get_size(list_2));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
	EXPECT_EQ(2, cplus_llist_get_size(list_3));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
	EXPECT_EQ(1, cplus_llist_get_size(list_3));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
	EXPECT_EQ(0, cplus_llist_get_size(list_3));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(3, cplus_llist_get_size(list_4));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(2, cplus_llist_get_size(list_4));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(1, cplus_llist_get_size(list_4));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(0, cplus_llist_get_size(list_4));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(4, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(3, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(2, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(1, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(0, cplus_llist_get_size(list_5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_5));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_pop_if, functionity)
{
	int32_t * data = CPLUS_NULL;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
	EXPECT_EQ(0, (int32_t)(*data));
	EXPECT_EQ(4, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
	EXPECT_EQ(1, (int32_t)(*data));
	EXPECT_EQ(3, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
	EXPECT_EQ(2, (int32_t)(*data));
	EXPECT_EQ(2, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
	EXPECT_EQ(3, (int32_t)(*data));
	EXPECT_EQ(1, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
	EXPECT_EQ(4, (int32_t)(*data));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_pop_if, bad_parameter)
{
	int32_t * data = CPLUS_NULL;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (data = (int32_t *)cplus_llist_pop_if(list, CPLUS_NULL, CPLUS_NULL))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num5))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_if, functionity)
{
	int32_t * data = CPLUS_NULL;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(6, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num0))));
	EXPECT_EQ(0, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num1))));
	EXPECT_EQ(1, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num2))));
	EXPECT_EQ(2, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num3))));
	EXPECT_EQ(3, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num4))));
	EXPECT_EQ(4, *data);
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_is_consist, functionity)
{
	cplus_llist list1 = CPLUS_NULL, list2 = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
	EXPECT_EQ(CPLUS_FALSE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
	EXPECT_EQ(CPLUS_FALSE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
	EXPECT_EQ(CPLUS_FALSE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
	EXPECT_EQ(CPLUS_TRUE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_front_s, sequence)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_front_s, bad_parameter)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_back_s, sequence)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_back_s, bad_parameter)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_back(list, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_at_s, push_randomly)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 2, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_pop_of_s, pop_randomly)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_of(list, 0)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_of(list, 3)));
	EXPECT_EQ(2, cplus_llist_get_size(list));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_index_if_s, functionity)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(0, cplus_llist_get_index_if(list, find_num, &num0));
	EXPECT_EQ(1, cplus_llist_get_index_if(list, find_num, &num1));
	EXPECT_EQ(2, cplus_llist_get_index_if(list, find_num, &num2));
	EXPECT_EQ(3, cplus_llist_get_index_if(list, find_num, &num3));
	EXPECT_EQ(4, cplus_llist_get_index_if(list, find_num, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_index_if_s, bad_parameter)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, CPLUS_NULL, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, find_num, &num5));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_front_s, sequence_with_mempool)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_prev_new_s(1024))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
}

TEST(cplus_llist_push_front_s, bad_parameter_with_mempool)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_prev_new_s(1024))));
	EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, CPLUS_NULL));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(CPLUS_LLIST_FOREACH_s, sequence)
{
	int32_t * data = CPLUS_NULL, value = 4;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	CPLUS_LLIST_FOREACH(list, data)
	{
		EXPECT_EQ(value, *data);
		value --;
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_set_if_s, functionity)
{
	cplus_llist list = CPLUS_NULL;
	cplus_llist list_0 = CPLUS_NULL, list_1 = CPLUS_NULL, list_2 = CPLUS_NULL;
	cplus_llist list_3 = CPLUS_NULL, list_4 = CPLUS_NULL, list_5 = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
	EXPECT_EQ(15, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(17, cplus_mgr_report());
#endif
	EXPECT_NE(CPLUS_TRUE, (CPLUS_NULL != (list_0 = cplus_llist_get_set_if(list, find_num, &num0))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_1 = cplus_llist_get_set_if(list, find_num, &num1))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_2 = cplus_llist_get_set_if(list, find_num, &num2))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_3 = cplus_llist_get_set_if(list, find_num, &num3))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_4 = cplus_llist_get_set_if(list, find_num, &num4))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list_5 = cplus_llist_get_set_if(list, find_num, &num5))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(1, cplus_llist_get_size(list_1));
	EXPECT_EQ(2, cplus_llist_get_size(list_2));
	EXPECT_EQ(3, cplus_llist_get_size(list_3));
	EXPECT_EQ(4, cplus_llist_get_size(list_4));
	EXPECT_EQ(5, cplus_llist_get_size(list_5));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list_1)));
	EXPECT_EQ(0, cplus_llist_get_size(list_1));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
	EXPECT_EQ(1, cplus_llist_get_size(list_2));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
	EXPECT_EQ(0, cplus_llist_get_size(list_2));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
	EXPECT_EQ(2, cplus_llist_get_size(list_3));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
	EXPECT_EQ(1, cplus_llist_get_size(list_3));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
	EXPECT_EQ(0, cplus_llist_get_size(list_3));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(3, cplus_llist_get_size(list_4));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(2, cplus_llist_get_size(list_4));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(1, cplus_llist_get_size(list_4));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
	EXPECT_EQ(0, cplus_llist_get_size(list_4));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(4, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(3, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(2, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(1, cplus_llist_get_size(list_5));
	EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
	EXPECT_EQ(0, cplus_llist_get_size(list_5));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_5));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_pop_if_s, functionity)
{
	int32_t * data = CPLUS_NULL;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
	EXPECT_EQ(0, (int32_t)(*data));
	EXPECT_EQ(4, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
	EXPECT_EQ(1, (int32_t)(*data));
	EXPECT_EQ(3, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
	EXPECT_EQ(2, (int32_t)(*data));
	EXPECT_EQ(2, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
	EXPECT_EQ(3, (int32_t)(*data));
	EXPECT_EQ(1, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
	EXPECT_EQ(4, (int32_t)(*data));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_FALSE, CPLUS_NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_pop_if_s, bad_parameter)
{
	int32_t * data = CPLUS_NULL;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (data = (int32_t *)cplus_llist_pop_if(list, CPLUS_NULL, CPLUS_NULL))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num5))));
	EXPECT_EQ(ENOENT, errno);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_if_s, functionity)
{
	int32_t * data = CPLUS_NULL;
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(7, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num0))));
	EXPECT_EQ(0, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num1))));
	EXPECT_EQ(1, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num2))));
	EXPECT_EQ(2, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num3))));
	EXPECT_EQ(3, *data);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num4))));
	EXPECT_EQ(4, *data);
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_is_consist_s, functionity)
{
	cplus_llist list1 = CPLUS_NULL, list2 = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
	EXPECT_EQ(CPLUS_FALSE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
	EXPECT_EQ(CPLUS_FALSE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
	EXPECT_EQ(CPLUS_FALSE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list1 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list2 = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
	EXPECT_EQ(CPLUS_TRUE, cplus_llist_is_consist(list1, list2, compare_num));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_delete_data, functionity)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num3));
	EXPECT_EQ(3, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_delete_data_s, functionity)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new_s())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num4));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(0, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

static int32_t ascendant(void * i1, void * i2)
{
	return *((int32_t *)i1) - *((int32_t *)i2);
}

static int32_t deascendant(void * i1, void * i2)
{
	return *((int32_t *)i2) - *((int32_t *)i1);
}

TEST(cplus_llist_sort, ascending)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, ascendant));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_sort, deascending)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, deascendant));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(CPLUS_TRUE, cplus_llist_is_sort(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_sort_s, ascending)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, ascendant));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(CPLUS_TRUE, cplus_llist_is_sort(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_sort_s, deascending)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, deascendant));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_push_back, pop_then_push)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
	EXPECT_EQ(3, cplus_llist_get_size(list));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 0)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 3)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 4)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_clear(list));
	EXPECT_EQ(0, cplus_llist_get_size(list));
#ifdef __CPLUS_MEM_MANAGER__
	EXPECT_EQ(1, cplus_mgr_report());
#endif
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_add_data, functionity)
{
	cplus_llist list = CPLUS_NULL;
	cplus_data data = CPLUS_NULL;
	int32_t i32_5 = 5, i32_6 = 6, i32_7 = 7, i32_8 = 8, i32_9 = 9;
	char * str1 = (char *)("Hello world"), * str2 = (char *)("AaBbCcDd");
	uint32_t str1_len = strlen(str1);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data_int32(list, "int_0", 0))));
	EXPECT_EQ(0, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data_int32(list, "int_1", 1))));
	EXPECT_EQ(1, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data_int32(list, "int_2", 2))));
	EXPECT_EQ(2, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data_int32(list, "int_3", 3))));
	EXPECT_EQ(3, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data_int32(list, "int_4", 4))));
	EXPECT_EQ(4, cplus_data_get_int32(data));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, cplus_data_get_int32(cplus_llist_find_data(list, "int_0")));
	EXPECT_EQ(1, cplus_data_get_int32(cplus_llist_find_data(list, "int_1")));
	EXPECT_EQ(2, cplus_data_get_int32(cplus_llist_find_data(list, "int_2")));
	EXPECT_EQ(3, cplus_data_get_int32(cplus_llist_find_data(list, "int_3")));
	EXPECT_EQ(4, cplus_data_get_int32(cplus_llist_find_data(list, "int_4")));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data(
			list, CPLUS_DATA_TYPE_INT32, "int_0", &i32_5, CPLUS_NULL))));
	EXPECT_EQ(5, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data(
			list, CPLUS_DATA_TYPE_INT32, "int_1", &i32_6, CPLUS_NULL))));
	EXPECT_EQ(6, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data(
			list, CPLUS_DATA_TYPE_INT32, "int_2", &i32_7, CPLUS_NULL))));
	EXPECT_EQ(7, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data(
			list, CPLUS_DATA_TYPE_INT32, "int_3", &i32_8, CPLUS_NULL))));
	EXPECT_EQ(8, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data(
			list, CPLUS_DATA_TYPE_INT32, "int_4", &i32_9, CPLUS_NULL))));
	EXPECT_EQ(9, cplus_data_get_int32(data));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(5, cplus_data_get_int32(cplus_llist_find_data(list, "int_0")));
	EXPECT_EQ(6, cplus_data_get_int32(cplus_llist_find_data(list, "int_1")));
	EXPECT_EQ(7, cplus_data_get_int32(cplus_llist_find_data(list, "int_2")));
	EXPECT_EQ(8, cplus_data_get_int32(cplus_llist_find_data(list, "int_3")));
	EXPECT_EQ(9, cplus_data_get_int32(cplus_llist_find_data(list, "int_4")));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_remove_data(list, "int_1"));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_remove_data(list,"int_3"));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == cplus_llist_find_data(list, "int_1")));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == cplus_llist_find_data(list, "int_3")));
	EXPECT_EQ(3, cplus_llist_get_size(list));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data(
			list, CPLUS_DATA_TYPE_STRING, "str1", &str1_len, (void *)str1))));
	EXPECT_EQ(0, strcmp(str1, cplus_data_get_string(data)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (data = cplus_llist_add_data_string(
			list, "str2", str2, strlen(str2)))));
	EXPECT_EQ(0, strcmp(str2, cplus_data_get_string(data)));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, strcmp(str1, cplus_data_get_string(cplus_llist_find_data(list, "str1"))));
	EXPECT_EQ(0, strcmp(str2, cplus_data_get_string(cplus_llist_find_data(list, "str2"))));
	EXPECT_EQ(strlen(str1), strlen(cplus_data_get_string(cplus_llist_find_data(list, "str1"))));
	EXPECT_EQ(strlen(str2), strlen(cplus_data_get_string(cplus_llist_find_data(list, "str2"))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32(0x1234)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != cplus_llist_update_data(list, "int_0", data));
	EXPECT_EQ(0x1234, cplus_data_get_int32(cplus_llist_find_data(list, "int_0")));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32(0x4321)));
	EXPECT_EQ(0x4321, cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != cplus_llist_update_data(list, "int_5", data));
	EXPECT_EQ(0x4321, cplus_data_get_int32(cplus_llist_find_data(list, "int_5")));
	CPLUS_LLIST_FOREACH(list, data)
	{
		EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_next, functionity)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_head(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_tail(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_next(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_next(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_next(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_next(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_prev(list)));
	EXPECT_EQ(3, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_prev(list)));
	EXPECT_EQ(2, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_prev(list)));
	EXPECT_EQ(1, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_prev(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_current(list)));
	EXPECT_EQ(0, *((int32_t *)cplus_llist_get_head(list)));
	EXPECT_EQ(4, *((int32_t *)cplus_llist_get_tail(list)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(CPLUS_LLIST_FOREACH, functionity)
{
	cplus_llist list = CPLUS_NULL;
	int32_t * target = 0, idx = 0;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	CPLUS_LLIST_FOREACH(list, target)
	{
		EXPECT_EQ(idx, *(target));
		EXPECT_EQ(idx, *((int32_t *)cplus_llist_get_current(list)));
		idx++;
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_llist_get_cycling_next, functionity)
{
	cplus_llist list = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (list = cplus_llist_new())));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
	EXPECT_EQ(5, cplus_llist_get_size(list));
	for (int32_t idx = 0; idx < 423; idx ++)
	{
		EXPECT_EQ((idx % 5), *((int32_t *)cplus_llist_get_cycling_next(list)));
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
	EXPECT_EQ(0, cplus_mgr_report());
}