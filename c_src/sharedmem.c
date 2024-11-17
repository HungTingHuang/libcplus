/******************************************************************
* @file: sharedmem.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <sys/mman.h>
#include <fcntl.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_sharedmem.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 0)
#define SHAREDMEM_MAX_SIZE (1 * GB)
#define SHAREDMEM_NAME_MAX_SIZE 63
#define SHAREDMEM_NAMED_PATTERN "/.cplus.shm_%s"
#define SHAREDMEM_NAMED_PATTERN_SIZE (sizeof(SHAREDMEM_NAMED_PATTERN) - 3)

struct shared_mem
{
    uint16_t type;
    int fd;
    void * addr;
    uint32_t size;
    volatile bool is_owner;
    pid_t owner_id;
    char name[SHAREDMEM_NAME_MAX_SIZE + 1];
};

bool cplus_sharedmem_is_owner(cplus_sharedmem obj)
{
    CHECK_OBJECT_TYPE(obj);

    return (((struct shared_mem *)(obj))->is_owner
        AND getpid() == ((struct shared_mem *)(obj))->owner_id);
}

int32_t cplus_sharedmem_get_size(cplus_sharedmem obj)
{
    CHECK_OBJECT_TYPE(obj);

    return ((struct shared_mem *)(obj))->size;
}

char * cplus_sharedmem_get_name(cplus_sharedmem obj)
{
    CHECK_OBJECT_TYPE(obj);

    return &(((struct shared_mem *)(obj))->name[strlen(SHAREDMEM_NAMED_PATTERN) - 2]);
}

void * cplus_sharedmem_alloc(cplus_sharedmem obj)
{
    CHECK_OBJECT_TYPE(obj);

    return ((struct shared_mem *)(obj))->addr;
}

int32_t cplus_sharedmem_delete(cplus_sharedmem obj)
{
    struct shared_mem * shmem = (struct shared_mem *)(obj);

    CHECK_OBJECT_TYPE(obj);
    shmem = (struct shared_mem *)(obj);

    if (MAP_FAILED != shmem->addr)
    {
        munmap(shmem->addr, shmem->size);
        shmem->addr = MAP_FAILED;
    }

    if (INVALID_FD != shmem->fd)
    {
        close(shmem->fd);
        shmem->fd = INVALID_FD;
    }

    if (true == shmem->is_owner AND getpid() == shmem->owner_id)
    {
        shm_unlink(shmem->name);
    }

    cplus_free(shmem);
    return CPLUS_SUCCESS;
}

static void * sharedmem_create(const char * name, uint32_t size)
{
    struct shared_mem * shmem = CPLUS_NULL;

    if ((shmem = (struct shared_mem *)cplus_malloc(sizeof(struct shared_mem))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(shmem);
        shmem->type = OBJ_TYPE;
        shmem->fd = INVALID_FD;
        shmem->addr = MAP_FAILED;
        shmem->size = size;
        shmem->is_owner = false;
        shmem->owner_id = -1;

        if (0 > cplus_str_printf(
            shmem->name
            , SHAREDMEM_NAME_MAX_SIZE
            , SHAREDMEM_NAMED_PATTERN
            , name))
        {
            goto exit;
        }

        shmem->fd = shm_open(
            shmem->name
            , O_CREAT | O_EXCL | O_RDWR
            , DEFFILEMODE);
        if (INVALID_FD == shmem->fd)
        {
            goto exit;
        }
        else
        {
            fchmod(shmem->fd, S_IRWXU | S_IRWXG | S_IRWXO);

            shmem->is_owner = true;
            shmem->owner_id = getpid();

            if (-1 == ftruncate(shmem->fd, shmem->size))
            {
                goto exit;
            }
        }

        shmem->addr = mmap(
            CPLUS_NULL
            , shmem->size
            , PROT_READ | PROT_WRITE
            , MAP_SHARED
            , shmem->fd
            , 0);
        if (MAP_FAILED == shmem->addr)
        {
            goto exit;
        }

        if (INVALID_FD != shmem->fd)
        {
            close(shmem->fd);
        }
    }
    else
    {
        errno = ENOMEM;
    }

    return shmem;
exit:
    cplus_sharedmem_delete(shmem);
    return CPLUS_NULL;
}

static void * sharedmem_open(const char * name)
{
    struct stat sm_stat;
    struct shared_mem * shmem = CPLUS_NULL;

    if ((shmem = (struct shared_mem *)cplus_malloc(sizeof(struct shared_mem))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(shmem);
        shmem->type = OBJ_TYPE;
        shmem->fd = INVALID_FD;
        shmem->addr = MAP_FAILED;
        shmem->size = 0;
        shmem->is_owner = false;
        shmem->owner_id = -1;

        if (0 > cplus_str_printf(
            shmem->name
            , SHAREDMEM_NAME_MAX_SIZE
            , SHAREDMEM_NAMED_PATTERN
            , name))
        {
            goto exit;
        }

        shmem->fd = shm_open(shmem->name, O_RDWR, DEFFILEMODE);
        if (INVALID_FD == shmem->fd)
        {
            goto exit;
        }
        else
        {
            shmem->is_owner = false;
            if (-1 == fstat(shmem->fd, &sm_stat))
            {
                goto exit;
            }
            shmem->size = sm_stat.st_size;
        }

        shmem->addr = mmap(
            CPLUS_NULL
            , shmem->size
            , PROT_READ | PROT_WRITE
            , MAP_SHARED
            , shmem->fd
            , 0);
        if (MAP_FAILED == shmem->addr)
        {
            goto exit;
        }

        if (INVALID_FD != shmem->fd)
        {
            close(shmem->fd);
        }
    }
    else
    {
        errno = ENOMEM;
    }

    return shmem;
exit:
    cplus_sharedmem_delete(shmem);
    return CPLUS_NULL;
}

static void * sharedmem_new(const char * name, uint32_t size)
{
    struct stat sm_stat;
    struct shared_mem * shmem = CPLUS_NULL;

    if ((shmem = (struct shared_mem *)cplus_malloc(sizeof(struct shared_mem))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(shmem);
        shmem->type = OBJ_TYPE;
        shmem->fd = INVALID_FD;
        shmem->addr = MAP_FAILED;
        shmem->size = size;
        shmem->is_owner = false;
        shmem->owner_id = -1;

        if (0 > cplus_str_printf(
            shmem->name
            , SHAREDMEM_NAME_MAX_SIZE
            , SHAREDMEM_NAMED_PATTERN
            , name))
        {
            goto exit;
        }

        shmem->fd = shm_open(shmem->name, O_RDWR, DEFFILEMODE);
        if (INVALID_FD != shmem->fd)
        {
            shmem->is_owner = false;
            if (-1 == fstat(shmem->fd, &sm_stat))
            {
                goto exit;
            }
            shmem->size = sm_stat.st_size;
        }
        else
        {
            if (errno == ENOENT)
            {
                shmem->fd = shm_open(
                    shmem->name
                    , O_CREAT | O_EXCL | O_RDWR
                    , DEFFILEMODE);
                if (INVALID_FD == shmem->fd)
                {
                    goto exit;
                }
                else
                {
                    fchmod(shmem->fd, S_IRWXU | S_IRWXG | S_IRWXO);

                    shmem->is_owner = true;
                    shmem->owner_id = getpid();

                    if (-1 == ftruncate(shmem->fd, shmem->size))
                    {
                        goto exit;
                    }
                }
            }
            else
            {
                goto exit;
            }
        }

        shmem->addr = mmap(
            CPLUS_NULL
            , shmem->size
            , PROT_READ | PROT_WRITE
            , MAP_SHARED
            , shmem->fd
            , 0);
        if (MAP_FAILED == shmem->addr)
        {
            goto exit;
        }

        if (INVALID_FD != shmem->fd)
        {
            close(shmem->fd);
        }
    }
    else
    {
        errno = ENOMEM;
    }

    return shmem;
exit:
    cplus_sharedmem_delete(shmem);
    return CPLUS_NULL;
}

cplus_sharedmem cplus_sharedmem_create(const char * name, uint32_t size)
{
    CHECK_NOT_NULL(name, CPLUS_NULL);
    CHECK_IF(SHAREDMEM_NAME_MAX_SIZE < (strlen(name) + SHAREDMEM_NAMED_PATTERN_SIZE)
        , CPLUS_NULL);
    CHECK_IN_INTERVAL(size, 1, SHAREDMEM_MAX_SIZE, CPLUS_NULL);
    return sharedmem_create(name, size);
}

cplus_sharedmem cplus_sharedmem_open(const char * name)
{
    CHECK_NOT_NULL(name, CPLUS_NULL);
    CHECK_IF(SHAREDMEM_NAME_MAX_SIZE < (strlen(name) + SHAREDMEM_NAMED_PATTERN_SIZE)
        , CPLUS_NULL);
    return sharedmem_open(name);
}

cplus_sharedmem cplus_sharedmem_new(const char * name, uint32_t size)
{
    CHECK_NOT_NULL(name, CPLUS_NULL);
    CHECK_IF(SHAREDMEM_NAME_MAX_SIZE < (strlen(name) + SHAREDMEM_NAMED_PATTERN_SIZE)
        , CPLUS_NULL);
    CHECK_IN_INTERVAL(size, 1, SHAREDMEM_MAX_SIZE, CPLUS_NULL);
    return sharedmem_new(name, size);
}

bool cplus_sharedmem_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#ifdef __CPLUS_UNITTEST__
#include "cplus_systime.h"

static char TEST_SHMEM_NAME[] = "test";

struct test_area
{
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint32_t d;
    char buffer[100];
};

CPLUS_UNIT_TEST(cplus_sharedmem_new, functionity)
{
    cplus_sharedmem server = CPLUS_NULL, client = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (server = cplus_sharedmem_new(TEST_SHMEM_NAME, sizeof(struct test_area)))));
    UNITTEST_EXPECT_EQ(true, cplus_sharedmem_is_owner(server));
    UNITTEST_EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(server));
    UNITTEST_EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(server)));
    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (client = cplus_sharedmem_new(TEST_SHMEM_NAME, sizeof(struct test_area)))));
    UNITTEST_EXPECT_EQ(2, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(false, cplus_sharedmem_is_owner(client));
    UNITTEST_EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(client));
    UNITTEST_EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(client)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(client));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_sharedmem_alloc, cross_process_communication)
{
    cplus_sharedmem server = CPLUS_NULL, client = CPLUS_NULL;
    struct test_area *server_data = CPLUS_NULL, *client_data = CPLUS_NULL;


    UNITTEST_EXPECT_EQ(false, (CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, sizeof(struct test_area)))));

    UNITTEST_EXPECT_EQ(true, cplus_sharedmem_is_owner(server));
    UNITTEST_EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(server));
    UNITTEST_EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(server)));

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (server_data = (struct test_area *)cplus_sharedmem_alloc(server)));

    UNITTEST_EXPECT_EQ(true, (0x01 == (server_data->a = 0x01)));
    UNITTEST_EXPECT_EQ(true, (0x2345 == (server_data->b = 0x2345)));
    UNITTEST_EXPECT_EQ(true, (0x6789abcd == (server_data->c = 0x6789abcd)));
    UNITTEST_EXPECT_EQ(true, (0x76543210 == (server_data->d = 0x76543210)));
    UNITTEST_EXPECT_EQ(strlen("Hello World"), cplus_str_printf(server_data->buffer, 100, "Hello World"));
    UNITTEST_EXPECT_EQ(1, cplus_mgr_report());

    if (0 != fork())
    {
    }
    else
    {
        UNITTEST_EXPECT_EQ(false, (CPLUS_NULL == (client = cplus_sharedmem_open(TEST_SHMEM_NAME))));

        UNITTEST_EXPECT_EQ(false, cplus_sharedmem_is_owner(client));
        UNITTEST_EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(client));
        UNITTEST_EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(client)));

        UNITTEST_EXPECT_EQ(false, (CPLUS_NULL == (client_data = (struct test_area *)cplus_sharedmem_alloc(client))));

        UNITTEST_EXPECT_EQ(true, (0x01 == client_data->a));
        UNITTEST_EXPECT_EQ(true, (0x2345 == client_data->b));
        UNITTEST_EXPECT_EQ(true, (0x6789abcd == client_data->c));
        UNITTEST_EXPECT_EQ(true, (0x76543210 == client_data->d));
        UNITTEST_EXPECT_EQ(0, strcmp(client_data->buffer, "Hello World"));

        UNITTEST_EXPECT_EQ(true, (0x10 == (client_data->a = 0x10)));
        UNITTEST_EXPECT_EQ(true, (0x5432 == (client_data->b = 0x5432)));
        UNITTEST_EXPECT_EQ(true, (0xdcba9876 == (client_data->c = 0xdcba9876)));
        UNITTEST_EXPECT_EQ(true, (0x01234567 == (client_data->d = 0x01234567)));
        UNITTEST_EXPECT_EQ(strlen("World Hello"), cplus_str_printf(client_data->buffer, 100, "World Hello"));
        UNITTEST_EXPECT_EQ(2, cplus_mgr_report());
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(client));
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
        UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
        return;
    }

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    UNITTEST_EXPECT_EQ(0x10, server_data->a);
    UNITTEST_EXPECT_EQ(0x5432, server_data->b);
    UNITTEST_EXPECT_EQ(0xdcba9876, server_data->c);
    UNITTEST_EXPECT_EQ(0x01234567, server_data->d);
    UNITTEST_EXPECT_EQ(0, strcmp(server_data->buffer, "World Hello"));

    UNITTEST_EXPECT_EQ(1, cplus_mgr_report());

    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_sharedmem_new, bad_parameter)
{
    cplus_sharedmem server = CPLUS_NULL;
    char bad_name[53] = "0123456789abcd0123456789abcd0123456789abcd0123456789";

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_new(CPLUS_NULL, 100)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_new("Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World", 100)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_new(bad_name, 100)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_new(TEST_SHMEM_NAME, 0)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_new(TEST_SHMEM_NAME, -9999)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_new(TEST_SHMEM_NAME, SHAREDMEM_MAX_SIZE + 1)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
}

CPLUS_UNIT_TEST(cplus_sharedmem_open, bad_parameter)
{
    cplus_sharedmem client = CPLUS_NULL;

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (client = cplus_sharedmem_open(CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (client = cplus_sharedmem_open(TEST_SHMEM_NAME)));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
}

CPLUS_UNIT_TEST(cplus_sharedmem_create, bad_parameter)
{
    cplus_sharedmem server = CPLUS_NULL, client = CPLUS_NULL;
    char bad_name[53] = "0123456789abcd0123456789abcd0123456789abcd0123456789";

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_create(CPLUS_NULL, 100)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_create("Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World", 100)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_create(bad_name, 100)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, 0)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, -9999)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, SHAREDMEM_MAX_SIZE + 1)));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (server = cplus_sharedmem_new(TEST_SHMEM_NAME, 100)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (client = cplus_sharedmem_create(TEST_SHMEM_NAME, 100)));
    UNITTEST_EXPECT_EQ(EEXIST, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_sharedmem(void)
{
    UNITTEST_ADD_TESTCASE(cplus_sharedmem_new, functionity);
    UNITTEST_ADD_TESTCASE(cplus_sharedmem_alloc, cross_process_communication);
    UNITTEST_ADD_TESTCASE(cplus_sharedmem_new, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_sharedmem_open, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_sharedmem_create, bad_parameter);
}

#endif //__CPLUS_UNITTEST__