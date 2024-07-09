#ifndef __CPLUS_FILE_H__
#define __CPLUS_FILE_H__
#include "cplus_typedef.h"
#define _GNU_SOURCE
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cplus_file_access
{
    CPLUS_FILE_ACCESS_NONE        = 0,
    CPLUS_FILE_ACCESS_ACCMODE     = O_ACCMODE,
    CPLUS_FILE_ACCESS_RDONLY      = O_RDONLY,
    CPLUS_FILE_ACCESS_WRONLY      = O_WRONLY,
    CPLUS_FILE_ACCESS_RDWR        = O_RDWR,
} CPLUS_FILE_ACCESS;

typedef enum cplus_file_flag
{
    CPLUS_FILE_FALG_NONE        = 0,
    CPLUS_FILE_FALG_CREATE      = O_CREAT,
    CPLUS_FILE_FALG_EXCL        = O_EXCL,
    CPLUS_FILE_FALG_NOCTTY      = O_NOCTTY,
    CPLUS_FILE_FALG_TRUNC       = O_TRUNC,
    CPLUS_FILE_FALG_APPEND      = O_APPEND,
    CPLUS_FILE_FALG_NONBLOCK    = O_NONBLOCK,
    CPLUS_FILE_FALG_SYNC        = O_SYNC,
    CPLUS_FILE_FALG_FASYNC      = FASYNC,
    // CPLUS_FILE_FALG_DIRECT      = O_DIRECT,
    // CPLUS_FILE_FALG_LARGEFILE   = O_LARGEFILE,
    CPLUS_FILE_FALG_DIRECTORY   = O_DIRECTORY,
    CPLUS_FILE_FALG_NOFOLLOW    = O_NOFOLLOW,
    // CPLUS_FILE_FALG_NOATIME     = O_NOATIME,
    CPLUS_FILE_FALG_CLOEXEC     = O_CLOEXEC,
    CPLUS_FILE_FALG_NDELAY      = O_NDELAY,
} CPLUS_FILE_FLAG;

typedef enum cplus_file_mode
{
    CPLUS_FILE_MODE_NONE        = 0,
    CPLUS_FILE_MODE_OWNER_RWX   = 0700,
    CPLUS_FILE_MODE_OWNER_READ  = 0400,
    CPLUS_FILE_MODE_OWNER_WRITE = 0200,
    CPLUS_FILE_MODE_OWNER_EXEC  = 0100,
    CPLUS_FILE_MODE_GROUP_RWX   = 0070,
    CPLUS_FILE_MODE_GROUP_READ  = 0040,
    CPLUS_FILE_MODE_GROUP_WRITE = 0020,
    CPLUS_FILE_MODE_GROUP_EXEC  = 0010,
    CPLUS_FILE_MODE_OTHER_RWX   = 0007,
    CPLUS_FILE_MODE_OTHER_READ  = 0004,
    CPLUS_FILE_MODE_OTHER_WRITE = 0002,
    CPLUS_FILE_MODE_OTHER_EXEC  = 0001,
    CPLUS_FILE_MODE_ALL         = 0777,
} CPLUS_FILE_MODE;

typedef enum cplus_file_pos
{
    CPLUS_FILE_POS_START    = SEEK_SET,
    CPLUS_FILE_POS_END      = SEEK_END,
    CPLUS_FILE_POS_CURR     = SEEK_CUR,
} CPLUS_FILE_POS;

bool cplus_file_is_file(char * file);
bool cplus_file_is_exist(char * file);
bool cplus_file_is_readable(char * file);
bool cplus_file_is_writable(char * file);
bool cplus_file_is_executable(char * file);

cplus_file cplus_file_new(char * file, CPLUS_FILE_ACCESS access);
cplus_file cplus_file_new_ex(char * file, CPLUS_FILE_ACCESS access, CPLUS_FILE_FLAG flag, CPLUS_FILE_MODE mode);
int32_t cplus_file_delete(cplus_file obj);
int32_t cplus_file_open(cplus_file obj);
int32_t cplus_file_close(cplus_file obj);
int32_t cplus_file_remove(char * file);
bool cplus_file_check(cplus_object obj);

int32_t cplus_file_write(cplus_file obj, uint32_t size, void * buffer);
int32_t cplus_file_read(cplus_file obj, uint32_t size, void * buffer);
int32_t cplus_file_sync(cplus_file obj);

int32_t cplus_file_get_fd(cplus_file obj);
int32_t cplus_file_get_data_size(cplus_file obj);
void * cplus_file_get_data(cplus_file obj);
char * cplus_file_get_file_path(cplus_file obj);
int32_t cplus_file_resize(cplus_file obj, uint32_t size);
int32_t cplus_file_move_pos(cplus_file obj, CPLUS_FILE_POS pos, int32_t offset);
int32_t cplus_file_reset_pos(cplus_file obj);
int32_t cplus_file_get_cur_pos(cplus_file obj);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_FILE_H__