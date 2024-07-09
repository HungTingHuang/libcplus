/******************************************************************
* @file: file.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include "common.h"
#include "cplus.h"
#include "cplus_file.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 5)

struct file
{
    uint16_t type;
    int32_t fd;
    char * file_path;
    uint32_t access;
    uint32_t flag;
    uint32_t mode;
    uint32_t data_bufs_size;
    void * data_bufs;
};

static inline bool is_file(char * file)
{
    struct stat st = {0};

    if (-1 == stat(file, &st))
    {
        return false;
    }
    return (st.st_mode & S_IFREG);
}

static inline bool is_exist(char * file)
{
    return (0 == access(file, F_OK))? true: false;
}

static inline bool is_readable(char * file)
{
    return (0 == access(file, R_OK))? true: false;
}

static inline bool is_writable(char * file)
{
    return (0 == access(file, W_OK))? true: false;
}

static inline bool is_executable(char * file)
{
    return (0 == access(file, X_OK))? true: false;
}

static inline int32_t get_size(struct file * f)
{
    int32_t size = 0, pos = 0;

    if (-1 != (pos = lseek(f->fd, 0L, SEEK_CUR)))
    {
        size = lseek(f->fd, 0L, SEEK_END);
        (void)lseek(f->fd, pos, SEEK_SET);
    }
    return size;
}

static inline int32_t reset_pos(struct file * f)
{
    return lseek(f->fd, 0, SEEK_SET);
}

static inline bool is_enough(uint32_t need_size)
{
    struct sysinfo info = {0};

    if (!sysinfo(&info))
    {
        if (need_size < info.freeram)
        {
            return true;
        }
    }
    return false;
}

static void * file_initialize_object(
    char * file
    , uint32_t access
    , uint32_t flag
    , uint32_t mode)
{
    struct file * f = NULL;

    if ((f = (struct file *)cplus_malloc(sizeof(struct file))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(f);

        f->type = OBJ_TYPE;
        f->access = access;
        f->flag = flag;
        f->mode = mode;
        f->fd = open(file, (f->access | f->flag), f->mode);
        if (-1 == f->fd)
        {
            goto exit;
        }

        if ((f->file_path = (char *)cplus_malloc(strlen(file) + 1)))
        {
            cplus_str_printf(f->file_path, strlen(file) + 1, "%s", file);
        }
    }

    return f;
exit:
    cplus_file_delete(f);
    return NULL;
}

cplus_file cplus_file_new_ex(
    char * file
    , CPLUS_FILE_ACCESS access
    , CPLUS_FILE_FLAG flag
    , CPLUS_FILE_MODE mode)
{
    return file_initialize_object(file, access, flag, mode);
}

cplus_file cplus_file_new(
    char * file
    , CPLUS_FILE_ACCESS access)
{
    if (!is_exist(file))
    {
        return NULL;
    }
    return file_initialize_object(file, access, CPLUS_FILE_FALG_NONE, CPLUS_FILE_MODE_ALL);
}

int32_t cplus_file_open(cplus_file obj)
{
    int32_t res = CPLUS_FAIL;
    struct file * f = (struct file *)(obj);
    uint32_t flag = 0;
    CHECK_OBJECT_TYPE(obj);

    flag = f->flag;
    if (-1 == f->fd)
    {
        if (!is_exist(f->file_path))
        {
            flag |= CPLUS_FILE_FALG_CREATE;
        }

        f->fd = open((const char *)(f->file_path), (f->access | flag), f->mode);
        if (-1 == f->fd)
        {
            goto exit;
        }
        res = CPLUS_SUCCESS;
    }
exit:
    return res;
}

int32_t cplus_file_close(cplus_file obj)
{
    int32_t res = CPLUS_FAIL;
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    if (0 == (res = close(f->fd)))
    {
        f->fd = -1;
    }
    return res;
}

int32_t cplus_file_delete(cplus_file obj)
{
    int32_t res = CPLUS_FAIL;
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    fsync(f->fd);

    if (0 == (res = close(f->fd)))
    {
        if (f->file_path)
        {
            cplus_free(f->file_path);
        }
        if (f->data_bufs)
        {
            cplus_free(f->data_bufs);
        }
        cplus_free(f);
    }
    return res;
}

int32_t cplus_file_remove(char * file)
{
    int32_t res = CPLUS_FAIL;
    CHECK_NOT_NULL(file, CPLUS_FAIL);

    if (is_exist(file))
    {
        res = (0 == unlink(file))? CPLUS_SUCCESS: CPLUS_FAIL;
    }
    return res;
}

int32_t cplus_file_read(cplus_file obj, uint32_t size, void * buffer)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(buffer, CPLUS_FAIL);
    CHECK_GT_ZERO(size, CPLUS_FAIL);

    return read(f->fd, buffer, size);
}

int32_t cplus_file_write(cplus_file obj, uint32_t size, void * buffer)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(buffer, CPLUS_FAIL);
    CHECK_GT_ZERO(size, CPLUS_FAIL);

    return write(f->fd, buffer, size);
}

int32_t cplus_file_sync(cplus_file obj)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    return fsync(f->fd);
}

int32_t cplus_file_resize(cplus_file obj, uint32_t size)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_GT_ZERO(size, CPLUS_FAIL);

    return ftruncate(f->fd, size);
}

int32_t cplus_file_move_pos(cplus_file obj, CPLUS_FILE_POS pos, int32_t offset)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    return lseek(f->fd, offset, pos);
}

int32_t cplus_file_get_cur_pos(cplus_file obj)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    return lseek(f->fd, 0L, SEEK_CUR);
}

int32_t cplus_file_reset_pos(cplus_file obj)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    return reset_pos(f);
}

int32_t cplus_file_get_data_size(cplus_file obj)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    return get_size(f);
}

void * cplus_file_get_data(cplus_file obj)
{
    int32_t f_size = 0, pos = 0;
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    f->data_bufs = NULL;

    if (0 < (f_size = get_size(f)))
    {
        f->data_bufs_size = (uint32_t)(f_size);

        if (!is_enough(f->data_bufs_size))
        {
            goto exit;
        }

        if (!(f->data_bufs = (void *)cplus_malloc(f->data_bufs_size)))
        {
            goto exit;
        }

        if (-1 != (pos = lseek(f->fd, 0L, SEEK_CUR)))
        {
            lseek(f->fd, 0, SEEK_SET);
            read(f->fd, f->data_bufs, f->data_bufs_size);
            lseek(f->fd, pos, SEEK_SET);
        }
    }

exit:
    return f->data_bufs;
}

char * cplus_file_get_file_path(cplus_file obj)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);

    return f->file_path;
}

bool cplus_file_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

int32_t cplus_file_get_fd(cplus_file obj)
{
    struct file * f = (struct file *)(obj);
    CHECK_OBJECT_TYPE(obj);
    return f->fd;
}

bool cplus_file_is_file(char * file)
{
    CHECK_NOT_NULL(file, CPLUS_FAIL);
    return is_file(file);
}

bool cplus_file_is_exist(char * file)
{
    CHECK_NOT_NULL(file, CPLUS_FAIL);
    return is_exist(file);
}

bool cplus_file_is_readable(char * file)
{
    CHECK_NOT_NULL(file, CPLUS_FAIL);
    return is_readable(file);
}

bool cplus_file_is_writable(char * file)
{
    CHECK_NOT_NULL(file, CPLUS_FAIL);
    return is_writable(file);
}

bool cplus_file_is_executable(char * file)
{
    return is_executable(file);
}

#ifdef __CPLUS_UNITTEST__

#define TEST_FILE   "/var/tmp/test"
#define TEST_STR    "Hello World"

CPLUS_UNIT_TEST(cplus_file_new, functionity)
{
    cplus_file file;

    UNITTEST_EXPECT_EQ(true, (NULL != (file = cplus_file_new_ex(
        TEST_FILE, CPLUS_FILE_ACCESS_RDWR, CPLUS_FILE_FALG_CREATE, CPLUS_FILE_MODE_ALL))));
    UNITTEST_EXPECT_EQ(true, (true == cplus_file_check(file)));
    UNITTEST_EXPECT_EQ(true, (true == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove(TEST_FILE));
    UNITTEST_EXPECT_EQ(true, (false == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_file_write, functionity)
{
    cplus_file file;

    UNITTEST_EXPECT_EQ(true, (NULL != (file = cplus_file_new_ex(
        TEST_FILE, CPLUS_FILE_ACCESS_WRONLY, CPLUS_FILE_FALG_CREATE, CPLUS_FILE_MODE_ALL))));
    UNITTEST_EXPECT_EQ(true, (true == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_write(file, strlen(TEST_STR), TEST_STR));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_file_read, functionity)
{
    cplus_file file;
    char rr[32] = {0};

    UNITTEST_EXPECT_EQ(true, (NULL != (file = cplus_file_new_ex(
        TEST_FILE, CPLUS_FILE_ACCESS_RDONLY, CPLUS_FILE_FALG_CREATE, CPLUS_FILE_MODE_ALL))));
    UNITTEST_EXPECT_EQ(true, (true == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_read(file, sizeof(rr), rr));
    UNITTEST_EXPECT_EQ(true, (0 == strcmp(TEST_STR, rr)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove(TEST_FILE));
    UNITTEST_EXPECT_EQ(true, (false == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_file_get_data_size, functionity)
{
    cplus_file file;

    UNITTEST_EXPECT_EQ(true, (NULL != (file = cplus_file_new_ex(
        TEST_FILE, CPLUS_FILE_ACCESS_WRONLY, CPLUS_FILE_FALG_CREATE, CPLUS_FILE_MODE_ALL))));
    UNITTEST_EXPECT_EQ(true, (true == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_write(file, strlen(TEST_STR), TEST_STR));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_get_data_size(file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove(TEST_FILE));
    UNITTEST_EXPECT_EQ(true, (false == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_file_get_data, functionity)
{
    cplus_file file;
    void * bufs = NULL;
    char rr[32] = {0};

    UNITTEST_EXPECT_EQ(true, (NULL != (file = cplus_file_new_ex(
        TEST_FILE, CPLUS_FILE_ACCESS_RDWR, CPLUS_FILE_FALG_CREATE, CPLUS_FILE_MODE_ALL))));
    UNITTEST_EXPECT_EQ(true, (true == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_write(file, strlen(TEST_STR), TEST_STR));
    UNITTEST_EXPECT_EQ(true, (NULL != (bufs = cplus_file_get_data(file))));
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, TEST_STR, cplus_file_get_data_size(file))));
    UNITTEST_EXPECT_EQ(0, cplus_file_reset_pos(file));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_read(file, sizeof(rr), rr));
    UNITTEST_EXPECT_EQ(true, (0 == strcmp(TEST_STR, rr)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove(TEST_FILE));
    UNITTEST_EXPECT_EQ(true, (false == cplus_file_is_exist(TEST_FILE)));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_file(void)
{
    UNITTEST_ADD_TESTCASE(cplus_file_new, functionity);
    UNITTEST_ADD_TESTCASE(cplus_file_write, functionity);
    UNITTEST_ADD_TESTCASE(cplus_file_read, functionity);
    UNITTEST_ADD_TESTCASE(cplus_file_get_data_size, functionity);
    UNITTEST_ADD_TESTCASE(cplus_file_get_data, functionity);
}

#endif // __CPLUS_UNITTEST__