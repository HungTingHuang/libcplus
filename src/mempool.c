/******************************************************************
* @file: mempool.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include "common.h"
#include "cplus.h"
#include "cplus_memmgr.h"
#include "cplus_mempool.h"
#include "cplus_rwlock.h"

#define OBJ_TYPE (OBJ_NONE + CORE + 1)

static uint8_t spin_up = 1;
static uint8_t spin_down = 0;

#define MEMPOOL_SPIN_LOCK() \
    do { while ((spin_up == cplus_atomic_read(&(mp->spinlock))) \
        || !cplus_atomic_compare_exchange(&(mp->spinlock), \
                &(spin_down), &(spin_up))) { } \
    } while (0)

#define MEMPOOL_SPIN_UNLOCK() \
    do { cplus_atomic_write(&(mp->spinlock), spin_down); } while(0)

struct mempool
{
    uint16_t type;
    uint32_t block_count;
    uint32_t block_size;
    uint32_t free_blocks_count;
    uint32_t initialized_blocks_count;
    void * mem_start;
    void * next_block; // next available memory block
    bool thread_safe;
    uint8_t spinlock;
};

static void * index_to_addr(struct mempool * mp, uint32_t index)
{
    return (void *)((uint8_t *)(mp->mem_start) + (mp->block_size * index));
}

static uint32_t addr_to_index(struct mempool * mp, void * addr)
{
    return ((uint32_t)((uint8_t *)(addr) - (uint8_t *)(mp->mem_start))) / mp->block_size;
}

uint32_t cplus_mempool_get_free_blocks_count(cplus_mempool obj)
{
    struct mempool * mp = (struct mempool *)(obj);
    uint32_t free_blocks_count = 0;
    CHECK_OBJECT_TYPE(obj);

    MEMPOOL_SPIN_LOCK();
    free_blocks_count = mp->free_blocks_count;
    MEMPOOL_SPIN_UNLOCK();

    return free_blocks_count;
}

int32_t cplus_mempool_free(cplus_mempool obj, void * addr)
{
    struct mempool * mp = (struct mempool *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(addr, CPLUS_FAIL);

    MEMPOOL_SPIN_LOCK();

    *((uint32_t *)(addr)) = (mp->next_block)
        ? addr_to_index(mp, mp->next_block)
        : mp->block_count;

    mp->next_block = addr;
    mp->free_blocks_count ++;

    MEMPOOL_SPIN_UNLOCK();

    return CPLUS_SUCCESS;
}

void * cplus_mempool_alloc(cplus_mempool obj)
{
    struct mempool * mp = (struct mempool *)(obj);
    void * addr = NULL;
    uint32_t * p = NULL;
    CHECK_OBJECT_TYPE(obj);

    // initialize
    MEMPOOL_SPIN_LOCK();
    if (mp->initialized_blocks_count < mp->block_count)
    {
        p = (uint32_t *)index_to_addr(mp, mp->initialized_blocks_count);
        *p = mp->initialized_blocks_count + 1;
        mp->initialized_blocks_count ++;
    }

    if (0 < mp->free_blocks_count)
    {
        addr = mp->next_block;
        mp->free_blocks_count --;

        mp->next_block = (0 < mp->free_blocks_count)
            ? (void *)index_to_addr(mp, *((uint32_t *)mp->next_block))
            : NULL;
    }
    else
    {
        errno = ENOMEM;
    }
    MEMPOOL_SPIN_UNLOCK();

    return addr;
}

uint32_t cplus_mempool_get_index(cplus_mempool obj, void * addr)
{
    uint32_t index = 0;
    struct mempool * mp = (struct mempool *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(addr, CPLUS_FAIL);

    MEMPOOL_SPIN_LOCK();
    index = addr_to_index(mp, addr);
    MEMPOOL_SPIN_UNLOCK();

    return index;
}

void * cplus_mempool_get_addr_by_index(cplus_mempool obj, uint32_t index)
{
    void * addr = NULL;
    struct mempool * mp = (struct mempool *)(obj);
    CHECK_OBJECT_TYPE(obj);

    MEMPOOL_SPIN_LOCK();
    addr = index_to_addr(mp, index);
    MEMPOOL_SPIN_UNLOCK();

    return addr;
}

uint32_t cplus_mempool_alloc_as_index(cplus_mempool obj)
{
    uint32_t index = 0;
    void * addr = NULL;
    struct mempool * mp = (struct mempool *)(obj);
    CHECK_OBJECT_TYPE(obj);

    if ((addr = (void *)cplus_mempool_alloc(obj)))
    {
        index = cplus_mempool_get_index(mp, addr);
    }
    return index;
}

int32_t cplus_mempool_free_by_index(cplus_mempool obj, uint32_t index)
{
    int32_t res = CPLUS_FAIL;
    struct mempool * mp = (struct mempool *)(obj);
    void * addr = NULL;
    CHECK_OBJECT_TYPE(obj);

    if ((addr = cplus_mempool_get_addr_by_index(obj, index)))
    {
        res = cplus_mempool_free(mp, addr);
    }
    return res;
}

int32_t cplus_mempool_delete(cplus_mempool obj)
{
    struct mempool * mp = (struct mempool *)(obj);
    CHECK_OBJECT_TYPE(obj);

    if (mp->mem_start)
    {
        cplus_free(mp->mem_start);
    }

    cplus_free(mp);
    return CPLUS_SUCCESS;
}

static void * mempool_initialize_object(
    uint32_t block_count
    , uint32_t block_size
    , bool thread_safe)
{
    struct mempool * mp = NULL;

    if ((mp = (struct mempool *)cplus_malloc(sizeof(struct mempool))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(mp);
        mp->type = OBJ_TYPE;
        mp->block_count = block_count;
        mp->block_size = block_size;
        mp->free_blocks_count = mp->block_count;
        mp->initialized_blocks_count = 0;
        mp->mem_start = (void *)cplus_malloc(mp->block_count * mp->block_size);
        if (NULL == mp->mem_start)
        {
            errno = ENOMEM;
            goto exit;
        }
        mp->next_block = mp->mem_start;
        mp->thread_safe = thread_safe;
        mp->spinlock = 0;
    }
    else
    {
        errno = ENOMEM;
    }

    return mp;
exit:
    cplus_mempool_delete(mp);
    return NULL;
}

cplus_mempool cplus_mempool_new(uint32_t block_count, uint32_t block_size)
{
    CHECK_IF(0 == block_count, NULL);
    CHECK_IF(sizeof(uint32_t) > block_size, NULL);
    return mempool_initialize_object(block_count, block_size, false);
}

cplus_mempool cplus_mempool_new_s(uint32_t block_count, uint32_t block_size)
{
    CHECK_IF(0 == block_count, NULL);
    CHECK_IF(sizeof(uint32_t) > block_size, NULL);
    return mempool_initialize_object(block_count, block_size, true);
}

bool cplus_mempool_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#ifdef __CPLUS_UNITTEST__
#include <string.h>

static int32_t *test0 = NULL, *test1 = NULL, *test2 = NULL, *test3 = NULL, *test4 = NULL;
static int32_t *test5 = NULL, *test6 = NULL, *test7 = NULL, *test8 = NULL, *test9 = NULL;

struct test_block
{
    uint8_t a; // size: 1
    uint16_t b; // size: 2
    uint32_t c; // size: 4
    uint32_t d; // size: 8
};

static struct test_block *test_block0 = NULL, *test_block1 = NULL, *test_block2 = NULL;

static uint8_t b10[] = {0x00}, b20[] = {0x00, 0x00}, b40[] = {0x00, 0x00, 0x00, 0x00}, b80[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t b1s[] = {0x01}, b2s[] = {0x01, 0x23}, b4s[] = {0x01, 0x23, 0x45, 0x67}, b8s[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
static uint8_t b1f[] = {0xff}, b2f[] = {0xff, 0xff}, b4f[] = {0xff, 0xff, 0xff, 0xff}, b8f[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

CPLUS_UNIT_TEST(cplus_mempool_alloc, functionity)
{
    cplus_mempool mp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new(5, sizeof(int)))));
    UNITTEST_EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test0 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test1 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test2 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test3 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test4 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(0, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL == (test5 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test6 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test7 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test8 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test9 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(0, (*test0 = 0));
    UNITTEST_EXPECT_EQ(1, (*test1 = 1));
    UNITTEST_EXPECT_EQ(2, (*test2 = 2));
    UNITTEST_EXPECT_EQ(3, (*test3 = 3));
    UNITTEST_EXPECT_EQ(4, (*test4 = 4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test0));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test1));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test2));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test3));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test4));
    UNITTEST_EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

CPLUS_UNIT_TEST(cplus_mempool_alloc, overlap)
{
    cplus_mempool mp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new(5, sizeof(int)))));
    UNITTEST_EXPECT_EQ(true, (NULL != (test0 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test1 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test2 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test3 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test4 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(0, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(0, (*test0 = 0));
    UNITTEST_EXPECT_EQ(1, (*test1 = 1));
    UNITTEST_EXPECT_EQ(2, (*test2 = 2));
    UNITTEST_EXPECT_EQ(3, (*test3 = 3));
    UNITTEST_EXPECT_EQ(4, (*test4 = 4));
    UNITTEST_EXPECT_EQ(0, *test0);
    UNITTEST_EXPECT_EQ(1, *test1);
    UNITTEST_EXPECT_EQ(2, *test2);
    UNITTEST_EXPECT_EQ(3, *test3);
    UNITTEST_EXPECT_EQ(4, *test4);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

CPLUS_UNIT_TEST(cplus_mempool_alloc, overlap_struct)
{
    cplus_mempool mp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new(5, sizeof(struct test_block)))));
    UNITTEST_EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test_block0 = (struct test_block *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test_block1 = (struct test_block *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test_block2 = (struct test_block *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->a, 0x00, sizeof(test_block0->a))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->b, 0x00, sizeof(test_block0->b))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->c, 0x00, sizeof(test_block0->c))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->d, 0x00, sizeof(test_block0->d))));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->a, b10, sizeof(test_block0->a)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->b, b20, sizeof(test_block0->b)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->c, b40, sizeof(test_block0->c)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->d, b80, sizeof(test_block0->d)));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->a, b1s, sizeof(test_block1->a))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->b, b2s, sizeof(test_block1->b))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->c, b4s, sizeof(test_block1->c))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->d, b8s, sizeof(test_block1->d))));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->a, b1s, sizeof(test_block1->a)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->b, b2s, sizeof(test_block1->b)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->c, b4s, sizeof(test_block1->c)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->d, b8s, sizeof(test_block1->d)));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->a, 0xff, sizeof(test_block2->a))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->b, 0xff, sizeof(test_block2->b))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->c, 0xff, sizeof(test_block2->c))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->d, 0xff, sizeof(test_block2->d))));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->a, b1f, sizeof(test_block2->a)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->b, b2f, sizeof(test_block2->b)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->c, b4f, sizeof(test_block2->c)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->d, b8f, sizeof(test_block2->d)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test_block0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test_block1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test_block2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

CPLUS_UNIT_TEST(cplus_mempool_new, bad_parameter)
{
    cplus_mempool mp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL == (mp = (cplus_mempool)cplus_mempool_new(0, 4))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 0))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 1))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 2))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (mp = (cplus_mempool)cplus_mempool_new(5, 3))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (mp = (cplus_mempool)cplus_mempool_new(3, GB))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
}

CPLUS_UNIT_TEST(cplus_mempool_alloc, no_mempool)
{
    UNITTEST_EXPECT_EQ(true, (NULL != (test_block0 = (struct test_block *)cplus_malloc(sizeof(struct test_block)))));
    UNITTEST_EXPECT_EQ(true, (NULL != (test_block1 = (struct test_block *)cplus_malloc(sizeof(struct test_block)))));
    UNITTEST_EXPECT_EQ(true, (NULL != (test_block2 = (struct test_block *)cplus_malloc(sizeof(struct test_block)))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->a, 0x00, sizeof(test_block0->a))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->b, 0x00, sizeof(test_block0->b))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->c, 0x00, sizeof(test_block0->c))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block0->d, 0x00, sizeof(test_block0->d))));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->a, b10, sizeof(test_block0->a)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->b, b20, sizeof(test_block0->b)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->c, b40, sizeof(test_block0->c)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block0->d, b80, sizeof(test_block0->d)));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->a, b1s, sizeof(test_block1->a))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->b, b2s, sizeof(test_block1->b))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->c, b4s, sizeof(test_block1->c))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_cpy(&test_block1->d, b8s, sizeof(test_block1->d))));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->a, b1s, sizeof(test_block1->a)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->b, b2s, sizeof(test_block1->b)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->c, b4s, sizeof(test_block1->c)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block1->d, b8s, sizeof(test_block1->d)));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->a, 0xff, sizeof(test_block2->a))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->b, 0xff, sizeof(test_block2->b))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->c, 0xff, sizeof(test_block2->c))));
    UNITTEST_EXPECT_EQ(true , (NULL != cplus_mem_set(&test_block2->d, 0xff, sizeof(test_block2->d))));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->a, b1f, sizeof(test_block2->a)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->b, b2f, sizeof(test_block2->b)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->c, b4f, sizeof(test_block2->c)));
    UNITTEST_EXPECT_EQ(0, memcmp(&test_block2->d, b8f, sizeof(test_block2->d)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(test_block0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(test_block1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(test_block2));
}

CPLUS_UNIT_TEST(cplus_mempool_new_s, functionity)
{
    cplus_mempool mp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(int)))));
    UNITTEST_EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test0 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test1 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test2 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test3 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL != (test4 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(0, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(true, (NULL == (test5 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test6 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test7 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test8 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (test9 = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(ENOMEM, errno);
    UNITTEST_EXPECT_EQ(0, (*test0 = 0));
    UNITTEST_EXPECT_EQ(1, (*test1 = 1));
    UNITTEST_EXPECT_EQ(2, (*test2 = 2));
    UNITTEST_EXPECT_EQ(3, (*test3 = 3));
    UNITTEST_EXPECT_EQ(4, (*test4 = 4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test0));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test1));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test2));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test3));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, test4));
    UNITTEST_EXPECT_EQ(5, cplus_mempool_get_free_blocks_count(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
}

CPLUS_UNIT_TEST(cplus_mempool_get_index, functionity)
{
    cplus_mempool mp = NULL;
    int32_t * addr[5], * new_addr = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
    for (int32_t idx = 0; idx < 5; idx++)
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (addr[idx] = (int32_t *)cplus_mempool_alloc(mp))));
    }
    UNITTEST_EXPECT_EQ(0, cplus_mempool_get_index(mp, addr[0]));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_index(mp, addr[1]));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_index(mp, addr[2]));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_index(mp, addr[3]));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_get_index(mp, addr[4]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, addr[2]));
    UNITTEST_EXPECT_EQ(true, (NULL != (new_addr = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_get_index(mp, new_addr));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, addr[1]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free(mp, addr[3]));
    UNITTEST_EXPECT_EQ(true, (NULL != (new_addr = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_get_index(mp, new_addr));
    UNITTEST_EXPECT_EQ(true, (NULL != (new_addr = (int32_t *)cplus_mempool_alloc(mp))));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_get_index(mp, new_addr));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mempool_get_addr_by_index, functionity)
{
    cplus_mempool mp = NULL;
    int32_t * addr[5];

    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
    for (int32_t idx = 0; idx < 5; idx++)
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (addr[idx] = (int32_t *)cplus_mempool_alloc(mp))));
    }
    UNITTEST_EXPECT_EQ(true, (addr[0] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 0))));
    UNITTEST_EXPECT_EQ(true, (addr[1] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 1))));
    UNITTEST_EXPECT_EQ(true, (addr[2] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 2))));
    UNITTEST_EXPECT_EQ(true, (addr[3] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 3))));
    UNITTEST_EXPECT_EQ(true, (addr[4] == ((int32_t *)cplus_mempool_get_addr_by_index(mp, 4))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mempool_alloc_as_index, functionity)
{
    cplus_mempool mp = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
    for (int32_t idx = 0; idx < 5; idx++)
    {
        UNITTEST_EXPECT_EQ(idx, cplus_mempool_alloc_as_index(mp));
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mempool_free_by_index, functionity)
{
    cplus_mempool mp = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (mp = (cplus_mempool)cplus_mempool_new_s(5, sizeof(uint32_t)))));
    for (int32_t idx = 0; idx < 5; idx++)
    {
        UNITTEST_EXPECT_EQ(idx, cplus_mempool_alloc_as_index(mp));
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 0));
    UNITTEST_EXPECT_EQ(0, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 3));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 1));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 4));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 2));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_free_by_index(mp, 4));
    UNITTEST_EXPECT_EQ(4, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(3, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(2, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(1, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(0, cplus_mempool_alloc_as_index(mp));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mempool_delete(mp));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_mempool(void)
{
    UNITTEST_ADD_TESTCASE(cplus_mempool_alloc, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mempool_alloc, overlap);
    UNITTEST_ADD_TESTCASE(cplus_mempool_alloc, overlap_struct);
    // UNITTEST_ADD_TESTCASE(cplus_mempool_new, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_mempool_alloc, no_mempool);
    UNITTEST_ADD_TESTCASE(cplus_mempool_new_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mempool_get_index, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mempool_get_addr_by_index, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mempool_alloc_as_index, functionity);
    UNITTEST_ADD_TESTCASE(cplus_mempool_free_by_index, functionity);
}
#endif //__CPLUS_UNITTEST__