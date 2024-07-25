#ifndef __CPLUS_SYSLOG_H__
#define __CPLUS_SYSLOG_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPLUS_SYSLOG_PRINT_ERROR(SYSLOG, REASON) \
    cplus_syslog_error(SYSLOG, "[%s](%d): Failed for '%s'\n", __FUNCTION__, __LINE__, #REASON);

#define CPLUS_SYSLOG_IFRV(SYSLOG, COND, VALUE) if ((COND)) { CPLUS_SYSLOG_PRINT_ERROR(SYSLOG, COND); return VALUE; }
#define CPLUS_SYSLOG_IFRN(SYSLOG, COND) if ((COND)) { CPLUS_SYSLOG_PRINT_ERROR(SYSLOG, COND); return; }
#define CPLUS_SYSLOG_IFBK(SYSLOG, COND) if ((COND)) { CPLUS_SYSLOG_PRINT_ERROR(SYSLOG, COND); break; }
#define CPLUS_SYSLOG_IFGT(SYSLOG, COND, TAG) if ((COND)) { CPLUS_SYSLOG_PRINT_ERROR(SYSLOG, COND); goto TAG; }

typedef enum cplus_debug_level
{
    CPLUS_SYSLOG_LEVEL_NONE,
    CPLUS_SYSLOG_LEVEL_FATAL,
    CPLUS_SYSLOG_LEVEL_ERROR,
    CPLUS_SYSLOG_LEVEL_WARN,
    CPLUS_SYSLOG_LEVEL_INFO,
    CPLUS_SYSLOG_LEVEL_DEBUG,
    CPLUS_SYSLOG_LEVEL_TRACE,
}CPLUS_DEBUG_LEVEL;

cplus_syslog cplus_syslog_new(void);
cplus_syslog cplus_syslog_new_ex(const char * custom_program_name);
int32_t cplus_syslog_delete(cplus_syslog obj);

bool cplus_syslog_check(cplus_object obj);
int32_t cplus_syslog_fatal(cplus_syslog obj, const char * format, ...);
int32_t cplus_syslog_error(cplus_syslog obj, const char * format, ...);
int32_t cplus_syslog_warn(cplus_syslog obj, const char * format, ...);
int32_t cplus_syslog_info(cplus_syslog obj, const char * format, ...);
int32_t cplus_syslog_debug(cplus_syslog obj, const char * format, ...);
int32_t cplus_syslog_trace(cplus_syslog obj, const char * format, ...);

CPLUS_DEBUG_LEVEL cplus_syslog_get_debug_level(cplus_syslog obj);
CPLUS_DEBUG_LEVEL cplus_syslog_get_logger_level(cplus_syslog obj);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_SYSLOG_H__