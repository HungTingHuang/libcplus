#include "internal.h"
#define TEST_FILE "/var/tmp/test"
#define TEST_STR "Hello World"

TEST(cplus_file_new, functionity)
{
	cplus_file file;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (file = cplus_file_new_ex(
		(char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR, CPLUS_FILE_FLAG_CREATE, CPLUS_FILE_MODE_ALL))));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_file_check(file)));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_TRUE, (false == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_file_write, functionity)
{
	cplus_file file;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (file = cplus_file_new_ex(
		(char *)(TEST_FILE), CPLUS_FILE_ACCESS_WRONLY, CPLUS_FILE_FLAG_CREATE, CPLUS_FILE_MODE_ALL))));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_write(file, strlen(TEST_STR), (void *)(TEST_STR)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_file_read, functionity)
{
	cplus_file file;
	char rr[32] = {0};
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (file = cplus_file_new_ex(
		(char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDONLY, CPLUS_FILE_FLAG_CREATE, CPLUS_FILE_MODE_ALL))));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_read(file, sizeof(rr), rr));
	EXPECT_EQ(CPLUS_TRUE, (0 == strcmp(TEST_STR, rr)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_TRUE, (false == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_file_get_data_size, functionity)
{
	cplus_file file;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (file = cplus_file_new_ex(
		(char *)(TEST_FILE), CPLUS_FILE_ACCESS_WRONLY, CPLUS_FILE_FLAG_CREATE, CPLUS_FILE_MODE_ALL))));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_write(file, strlen(TEST_STR), (void *)(TEST_STR)));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_get_data_size(file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_TRUE, (false == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_file_get_data, functionity)
{
	cplus_file file;
	void * bufs = CPLUS_NULL;
	char rr[32] = {0};
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (file = cplus_file_new_ex(
		(char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR, CPLUS_FILE_FLAG_CREATE, CPLUS_FILE_MODE_ALL))));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_write(file, strlen(TEST_STR), (void *)(TEST_STR)));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (bufs = cplus_file_get_data(file))));
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, TEST_STR, cplus_file_get_data_size(file))));
	EXPECT_EQ(0, cplus_file_reset_pos(file));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_read(file, sizeof(rr), rr));
	EXPECT_EQ(CPLUS_TRUE, (0 == strcmp(TEST_STR, rr)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_TRUE, (false == cplus_file_is_exist((char *)(TEST_FILE))));
	EXPECT_EQ(0, cplus_mgr_report());
}