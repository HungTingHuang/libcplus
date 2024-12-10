#include "internal.h"
#define SHAREDMEM_MAX_SIZE (1 * 1073741824)

static char TEST_SHMEM_NAME[] = "test";
struct test_area
{
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint32_t d;
    char buffer[100];
};

TEST(cplus_sharedmem_new, functionity)
{
    cplus_sharedmem server = CPLUS_NULL, client = CPLUS_NULL;
    EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (server = cplus_sharedmem_new(TEST_SHMEM_NAME, sizeof(struct test_area)))));
    EXPECT_EQ(CPLUS_TRUE, cplus_sharedmem_is_owner(server));
    EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(server));
    EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(server)));
    EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (client = cplus_sharedmem_new(TEST_SHMEM_NAME, sizeof(struct test_area)))));
    EXPECT_EQ(2, cplus_mgr_report());
    EXPECT_EQ(CPLUS_FALSE, cplus_sharedmem_is_owner(client));
    EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(client));
    EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(client)));
    EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
    EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(client));
    EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_sharedmem_alloc, cross_process_communication)
{
    cplus_sharedmem server = CPLUS_NULL, client = CPLUS_NULL;
    struct test_area *server_data = CPLUS_NULL, *client_data = CPLUS_NULL;
    EXPECT_EQ(CPLUS_FALSE, (CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, sizeof(struct test_area)))));
    EXPECT_EQ(CPLUS_TRUE, cplus_sharedmem_is_owner(server));
    EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(server));
    EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(server)));
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (server_data = (struct test_area *)cplus_sharedmem_alloc(server)));
    EXPECT_EQ(CPLUS_TRUE, (0x01 == (server_data->a = 0x01)));
    EXPECT_EQ(CPLUS_TRUE, (0x2345 == (server_data->b = 0x2345)));
    EXPECT_EQ(CPLUS_TRUE, (0x6789abcd == (server_data->c = 0x6789abcd)));
    EXPECT_EQ(CPLUS_TRUE, (0x76543210 == (server_data->d = 0x76543210)));
    EXPECT_EQ(strlen("Hello World"), cplus_str_printf(server_data->buffer, 100, "Hello World"));
    EXPECT_EQ(1, cplus_mgr_report());
    if (0 != fork())
    {
    }
    else
    {
        EXPECT_EQ(CPLUS_FALSE, (CPLUS_NULL == (client = cplus_sharedmem_open(TEST_SHMEM_NAME))));
        EXPECT_EQ(CPLUS_FALSE, cplus_sharedmem_is_owner(client));
        EXPECT_EQ(sizeof(struct test_area), cplus_sharedmem_get_size(client));
        EXPECT_EQ(0, strcmp(TEST_SHMEM_NAME, cplus_sharedmem_get_name(client)));
        EXPECT_EQ(CPLUS_FALSE, (CPLUS_NULL == (client_data = (struct test_area *)cplus_sharedmem_alloc(client))));
        EXPECT_EQ(CPLUS_TRUE, (0x01 == client_data->a));
        EXPECT_EQ(CPLUS_TRUE, (0x2345 == client_data->b));
        EXPECT_EQ(CPLUS_TRUE, (0x6789abcd == client_data->c));
        EXPECT_EQ(CPLUS_TRUE, (0x76543210 == client_data->d));
        EXPECT_EQ(0, strcmp(client_data->buffer, "Hello World"));
        EXPECT_EQ(CPLUS_TRUE, (0x10 == (client_data->a = 0x10)));
        EXPECT_EQ(CPLUS_TRUE, (0x5432 == (client_data->b = 0x5432)));
        EXPECT_EQ(CPLUS_TRUE, (0xdcba9876 == (client_data->c = 0xdcba9876)));
        EXPECT_EQ(CPLUS_TRUE, (0x01234567 == (client_data->d = 0x01234567)));
        EXPECT_EQ(strlen("World Hello"), cplus_str_printf(client_data->buffer, 100, "World Hello"));
        EXPECT_EQ(2, cplus_mgr_report());
        EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(client));
        EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
        EXPECT_EQ(0, cplus_mgr_report());
        return;
    }
    EXPECT_EQ(CPLUS_SUCCESS, unittest_wait_child_proc_end(5000));
    EXPECT_EQ(0x10, server_data->a);
    EXPECT_EQ(0x5432, server_data->b);
    EXPECT_EQ(0xdcba9876, server_data->c);
    EXPECT_EQ(0x01234567, server_data->d);
    EXPECT_EQ(0, strcmp(server_data->buffer, "World Hello"));
    EXPECT_EQ(1, cplus_mgr_report());
    EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
    EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_sharedmem_new, bad_parameter)
{
    cplus_sharedmem server = CPLUS_NULL;
    char bad_name[53] = "0123456789abcd0123456789abcd0123456789abcd0123456789";
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_new(CPLUS_NULL, 100)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_new("Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World", 100)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_new(bad_name, 100)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_new(TEST_SHMEM_NAME, 0)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_new(TEST_SHMEM_NAME, -9999)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_new(TEST_SHMEM_NAME, SHAREDMEM_MAX_SIZE + 1)));
    EXPECT_EQ(EINVAL, errno);
}

TEST(cplus_sharedmem_open, bad_parameter)
{
    cplus_sharedmem client = CPLUS_NULL;
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (client = cplus_sharedmem_open(CPLUS_NULL)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (client = cplus_sharedmem_open(TEST_SHMEM_NAME)));
    EXPECT_EQ(ENOENT, errno);
}

TEST(cplus_sharedmem_create, bad_parameter)
{
    cplus_sharedmem server = CPLUS_NULL, client = CPLUS_NULL;
    char bad_name[53] = "0123456789abcd0123456789abcd0123456789abcd0123456789";
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_create(CPLUS_NULL, 100)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_create("Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World Hello World", 100)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_create(bad_name, 100)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, 0)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, -9999)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (server = cplus_sharedmem_create(TEST_SHMEM_NAME, SHAREDMEM_MAX_SIZE + 1)));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (server = cplus_sharedmem_new(TEST_SHMEM_NAME, 100)));
    EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL == (client = cplus_sharedmem_create(TEST_SHMEM_NAME, 100)));
    EXPECT_EQ(EEXIST, errno);
    EXPECT_EQ(CPLUS_SUCCESS, cplus_sharedmem_delete(server));
    EXPECT_EQ(0, cplus_mgr_report());
}