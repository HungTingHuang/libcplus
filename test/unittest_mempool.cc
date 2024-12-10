#include "internal.h"

static int32_t *test0 = CPLUS_NULL, *test1 = CPLUS_NULL, *test2 = CPLUS_NULL, *test3 = CPLUS_NULL, *test4 = CPLUS_NULL;
static int32_t *test5 = CPLUS_NULL, *test6 = CPLUS_NULL, *test7 = CPLUS_NULL, *test8 = CPLUS_NULL, *test9 = CPLUS_NULL;

struct test_block
{
	uint8_t a; // size: 1
	uint16_t b; // size: 2
	uint32_t c; // size: 4
	uint32_t d; // size: 8
};

static struct test_block *test_block0 = CPLUS_NULL, *test_block1 = CPLUS_NULL, *test_block2 = CPLUS_NULL;

static uint8_t b10[] = {0x00}, b20[] = {0x00, 0x00}, b40[] = {0x00, 0x00, 0x00, 0x00}, b80[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t b1s[] = {0x01}, b2s[] = {0x01, 0x23}, b4s[] = {0x01, 0x23, 0x45, 0x67}, b8s[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
static uint8_t b1f[] = {0xff}, b2f[] = {0xff, 0xff}, b4f[] = {0xff, 0xff, 0xff, 0xff}, b8f[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

TEST(cplus_mempool_alloc, functionity)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new(5, sizeof(int)))));
	EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test0 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test1 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test2 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test3 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test4 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(0, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test5 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test6 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test7 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test8 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test9 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(0, (*test0 = 0));
	EXPECT_EQ(1, (*test1 = 1));
	EXPECT_EQ(2, (*test2 = 2));
	EXPECT_EQ(3, (*test3 = 3));
	EXPECT_EQ(4, (*test4 = 4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test0));
	EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test1));
	EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test2));
	EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test3));
	EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test4));
	EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

TEST(cplus_mempool_alloc, overlap)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new(5, sizeof(int)))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test0 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test1 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test2 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test3 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test4 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(0, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(0, (*test0 = 0));
	EXPECT_EQ(1, (*test1 = 1));
	EXPECT_EQ(2, (*test2 = 2));
	EXPECT_EQ(3, (*test3 = 3));
	EXPECT_EQ(4, (*test4 = 4));
	EXPECT_EQ(0, *test0);
	EXPECT_EQ(1, *test1);
	EXPECT_EQ(2, *test2);
	EXPECT_EQ(3, *test3);
	EXPECT_EQ(4, *test4);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

TEST(cplus_mempool_alloc, overlap_struct)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new(5, sizeof(struct test_block)))));
	EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_block0 = (struct test_block *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_block1 = (struct test_block *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_block2 = (struct test_block *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->a, 0x00, sizeof(test_block0->a))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->b, 0x00, sizeof(test_block0->b))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->c, 0x00, sizeof(test_block0->c))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->d, 0x00, sizeof(test_block0->d))));
	EXPECT_EQ(0, memcmp(&test_block0->a, b10, sizeof(test_block0->a)));
	EXPECT_EQ(0, memcmp(&test_block0->b, b20, sizeof(test_block0->b)));
	EXPECT_EQ(0, memcmp(&test_block0->c, b40, sizeof(test_block0->c)));
	EXPECT_EQ(0, memcmp(&test_block0->d, b80, sizeof(test_block0->d)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->a, b1s, sizeof(test_block1->a))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->b, b2s, sizeof(test_block1->b))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->c, b4s, sizeof(test_block1->c))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->d, b8s, sizeof(test_block1->d))));
	EXPECT_EQ(0, memcmp(&test_block1->a, b1s, sizeof(test_block1->a)));
	EXPECT_EQ(0, memcmp(&test_block1->b, b2s, sizeof(test_block1->b)));
	EXPECT_EQ(0, memcmp(&test_block1->c, b4s, sizeof(test_block1->c)));
	EXPECT_EQ(0, memcmp(&test_block1->d, b8s, sizeof(test_block1->d)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->a, 0xff, sizeof(test_block2->a))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->b, 0xff, sizeof(test_block2->b))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->c, 0xff, sizeof(test_block2->c))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->d, 0xff, sizeof(test_block2->d))));
	EXPECT_EQ(0, memcmp(&test_block2->a, b1f, sizeof(test_block2->a)));
	EXPECT_EQ(0, memcmp(&test_block2->b, b2f, sizeof(test_block2->b)));
	EXPECT_EQ(0, memcmp(&test_block2->c, b4f, sizeof(test_block2->c)));
	EXPECT_EQ(0, memcmp(&test_block2->d, b8f, sizeof(test_block2->d)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test_block0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test_block1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test_block2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

TEST(cplus_mempool_new, bad_parameter)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mp = (cplus_mempool)cplus_mempool_new(0, 4))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 0))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 1))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 2))));
	EXPECT_EQ(EINVAL, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 3))));
	EXPECT_EQ(EINVAL, errno);
	// EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (mp = (cplus_mempool)cplus_mempool_new(3, 1073741824))));
	// EXPECT_EQ(ENOMEM, errno);
}

TEST(cplus_mempool_alloc, no_mempool)
{
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_block0 = (struct test_block *)cplus_malloc(sizeof(struct test_block)))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_block1 = (struct test_block *)cplus_malloc(sizeof(struct test_block)))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_block2 = (struct test_block *)cplus_malloc(sizeof(struct test_block)))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->a, 0x00, sizeof(test_block0->a))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->b, 0x00, sizeof(test_block0->b))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->c, 0x00, sizeof(test_block0->c))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block0->d, 0x00, sizeof(test_block0->d))));
	EXPECT_EQ(0, memcmp(&test_block0->a, b10, sizeof(test_block0->a)));
	EXPECT_EQ(0, memcmp(&test_block0->b, b20, sizeof(test_block0->b)));
	EXPECT_EQ(0, memcmp(&test_block0->c, b40, sizeof(test_block0->c)));
	EXPECT_EQ(0, memcmp(&test_block0->d, b80, sizeof(test_block0->d)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->a, b1s, sizeof(test_block1->a))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->b, b2s, sizeof(test_block1->b))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->c, b4s, sizeof(test_block1->c))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_cpy(&test_block1->d, b8s, sizeof(test_block1->d))));
	EXPECT_EQ(0, memcmp(&test_block1->a, b1s, sizeof(test_block1->a)));
	EXPECT_EQ(0, memcmp(&test_block1->b, b2s, sizeof(test_block1->b)));
	EXPECT_EQ(0, memcmp(&test_block1->c, b4s, sizeof(test_block1->c)));
	EXPECT_EQ(0, memcmp(&test_block1->d, b8s, sizeof(test_block1->d)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->a, 0xff, sizeof(test_block2->a))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->b, 0xff, sizeof(test_block2->b))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->c, 0xff, sizeof(test_block2->c))));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != cplus_mem_set(&test_block2->d, 0xff, sizeof(test_block2->d))));
	EXPECT_EQ(0, memcmp(&test_block2->a, b1f, sizeof(test_block2->a)));
	EXPECT_EQ(0, memcmp(&test_block2->b, b2f, sizeof(test_block2->b)));
	EXPECT_EQ(0, memcmp(&test_block2->c, b4f, sizeof(test_block2->c)));
	EXPECT_EQ(0, memcmp(&test_block2->d, b8f, sizeof(test_block2->d)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(test_block0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(test_block1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_free(test_block2));
}

TEST(cplus_mempool_new_s, functionity)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(int)))));
	EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test0 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test1 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test2 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test3 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test4 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(0, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test5 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test6 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test7 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test8 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL == (test9 = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(ENOMEM, errno);
	EXPECT_EQ(0, (*test0 = 0));
	EXPECT_EQ(1, (*test1 = 1));
	EXPECT_EQ(2, (*test2 = 2));
	EXPECT_EQ(3, (*test3 = 3));
	EXPECT_EQ(4, (*test4 = 4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test0));
	EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test1));
	EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test2));
	EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test3));
	EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test4));
	EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

TEST(cplus_mempool_get_index, functionity)
{
	cplus_mempool mp = CPLUS_NULL;
	int32_t * addr[5], * new_addr = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
	for (int32_t idx = 0; idx < 5; idx++)
	{
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (addr[idx] = (int32_t *)cplus_mempool_alloc(mp))));
	}
	EXPECT_EQ(0, cplus_mempool_get_index(mp, addr[0]));
	EXPECT_EQ(1, cplus_mempool_get_index(mp, addr[1]));
	EXPECT_EQ(2, cplus_mempool_get_index(mp, addr[2]));
	EXPECT_EQ(3, cplus_mempool_get_index(mp, addr[3]));
	EXPECT_EQ(4, cplus_mempool_get_index(mp, addr[4]));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, addr[2]));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (new_addr = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(2, cplus_mempool_get_index(mp, new_addr));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, addr[1]));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, addr[3]));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (new_addr = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(3, cplus_mempool_get_index(mp, new_addr));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (new_addr = (int32_t *)cplus_mempool_alloc(mp))));
	EXPECT_EQ(1, cplus_mempool_get_index(mp, new_addr));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mempool_get_addr_by_index, functionity)
{
	cplus_mempool mp = CPLUS_NULL;
	int32_t * addr[5];
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
	for (int32_t idx = 0; idx < 5; idx++)
	{
		EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (addr[idx] = (int32_t *)cplus_mempool_alloc(mp))));
	}
	EXPECT_EQ(CPLUS_TRUE, (addr[0] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 0))));
	EXPECT_EQ(CPLUS_TRUE, (addr[1] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 1))));
	EXPECT_EQ(CPLUS_TRUE, (addr[2] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 2))));
	EXPECT_EQ(CPLUS_TRUE, (addr[3] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 3))));
	EXPECT_EQ(CPLUS_TRUE, (addr[4] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 4))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mempool_alloc_as_index, functionity)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
	for (int32_t idx = 0; idx < 5; idx++)
	{
		EXPECT_EQ(idx, cplus_mempool_alloc_as_index(mp));
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_mempool_free_by_index, functionity)
{
	cplus_mempool mp = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
	for (int32_t idx = 0; idx < 5; idx++)
	{
		EXPECT_EQ(idx, cplus_mempool_alloc_as_index(mp));
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 0));
	EXPECT_EQ(0, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 3));
	EXPECT_EQ(3, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 1));
	EXPECT_EQ(1, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 4));
	EXPECT_EQ(4, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 2));
	EXPECT_EQ(2, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 4));
	EXPECT_EQ(4, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(3, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(2, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(1, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(0, cplus_mempool_alloc_as_index(mp));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
	EXPECT_EQ(0, cplus_mgr_report());
}