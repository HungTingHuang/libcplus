/******************************************************************
* @file: syslog.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <syslog.h>
#include <stdarg.h>
#include <fcntl.h>
#include "common.h"
#include "cplus_syslog.h"
#include "cplus_memmgr.h"
#include "cplus_mempool.h"
#include "cplus_systime.h"
#include "cplus_task.h"
#include "cplus_llist.h"
#include "cplus_mutex.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 4)

#define MAX_LOG_FILE_COUNT 5
#define MAX_LOG_LENGTH 1024

#define MAX_PROGRAM_NAME_SIZE 31
#define MAX_MODULE_NAME_SIZE 61
#define MAX_FILE_PATH_SIZE 127
#define MAX_FILE_FOLDER_PATH_SIZE 31
#define MAX_MESSAGE_BUFFER_SIZE 1023
#define MAX_MESSAGE_COUNT 512
#define MAX_DEBUG_LEVEL_ENVNAME_SIZE 63
#define MAX_LOGGER_LEVEL_ENVNAME_SIZE 63

#define DEFAULT_ENABEL_COLOR 1
#define DEFAULT_DEBUG_LEVEL "4"
#define DEFAULT_LOGGER_LEVEL "0"
#define DEFAULT_FILE_STORE_LOCATION "/tmp/log"
#define DEFAULT_FILE_NAME "device.log"
#define BUILTIN_MODULE_NAME "unknown"
#define DEFAULT_FILE_MODE (O_WRONLY | O_APPEND | O_CREAT)
#define LOG_TIMESTAMP_FORMAT "%s %02d %02d:%02d:%02d.%03d%03d %s "
#define DEBUD_COLOR_MSG_FORMAT GREEN("[%02d:%02d:%02d.%03d%03d] ")YELLOW("%s")": %s "
#define DEBUD_MSG_FORMAT "[%02d:%02d:%02d.%03d%03d] %s: %s "

#define SUPPORT_SYSLOG 0

extern char * program_invocation_short_name;

static const char * debug_level_strings[] = {
"", "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"};

static const char * debug_level_color_strings[] = {
"", RED("FATAL"), RED("ERROR"), PURPLE("WARN "), BLUE("INFO "), "DEBUG", "TRACE"};

static const char * month_strings[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
"Sep", "Oct", "Nov", "Dec"};

static const char debug_level_envname_format[] = "cplus_syslog_%s_debug_level";
static const char logger_level_envname_format[] = "cplus_syslog_%s_logger_level";

struct syslog
{
	uint16_t type;
	int32_t debug_level;
	int32_t logger_level;
	bool enable_color;
	cplus_mempool message_buffer_pool;
	cplus_task message_logger_wroker;
	cplus_llist message_list;
	cplus_mutex message_sect;
	char debug_level_envname[MAX_DEBUG_LEVEL_ENVNAME_SIZE + 1];
	char logger_level_envname[MAX_LOGGER_LEVEL_ENVNAME_SIZE + 1];
	char program_name[MAX_PROGRAM_NAME_SIZE + 1];
	char file_path[MAX_FILE_PATH_SIZE + 1];
	char file_folder_path[MAX_FILE_FOLDER_PATH_SIZE + 1];
};

static void append_new_line(const char * str)
{
	if ('\n' != str[strlen(str) - 1])
	{
		fprintf(stdout, "%c", '\n');
	}
}

int32_t cplus_syslog_delete(cplus_syslog obj)
{
	int32_t res = CPLUS_SUCCESS;
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	if (slog->message_logger_wroker)
	{
		cplus_task_stop(slog->message_logger_wroker, 30 * 1000);
	}

	if (slog->message_buffer_pool)
	{
		cplus_mempool_delete(slog->message_buffer_pool);
	}
	if (slog->message_list)
	{
		cplus_llist_delete(slog->message_list);
	}
	if (slog->message_sect)
	{
		cplus_mutex_delete(slog->message_sect);
	}

	// unsetenv(slog->logger_level_envname);
	// unsetenv(slog->debug_level_envname);

	if (CPLUS_SYSLOG_LEVEL_DEBUG < slog->logger_level)
	{
		closelog();
	}

	cplus_free(slog);
	return res;
}

void custom_logger_executer(void * param1, void * param2)
{
	struct syslog * slog = (struct syslog *)param1;
	UNUSED_PARAM(param2);

	cplus_crit_sect_enter(slog->message_sect);
	{
		if (0 < cplus_llist_get_size(slog->message_list))
		{
			cplus_systime stime;
			cplus_systime_get_local_time(&stime);

			/*  Reason for False Positive : There is no overlapping copy because
				Maximum number of bytes in slog->file_path is 128,
				and format string will consume at most 78 (32 + 1 + 32 + 1 + 2 + 2 + 4 + 4) bytes */
			/* coverity[overlapping_copy: FALSE] */
			if (0 < cplus_str_printf(
				slog->file_path
				, MAX_FILE_PATH_SIZE
				, "%.*s/%.*s_%02d%02d%04d.log"
				, ((int32_t)sizeof(slog->file_folder_path))
				, slog->file_folder_path
				, ((int32_t)sizeof(slog->program_name))
				, slog->program_name
				, stime.month
				, stime.day_of_month
				, stime.year))
			{
				int fd = open(slog->file_path, DEFAULT_FILE_MODE, 0666);
				if (0 <= fd)
				{
					char * msg_buf = CPLUS_NULL;
					while (CPLUS_NULL != (msg_buf = (char *)cplus_llist_pop_back(slog->message_list)))
					{
						write(fd, msg_buf, strlen(msg_buf));
						if ('\n' != msg_buf[strlen(msg_buf) - 1])
						{
							write(fd, "\n", strlen("\n"));
						}
						cplus_mempool_free(slog->message_buffer_pool, msg_buf);
					}
					fsync(fd); close(fd);
				}
			}
		}
	}
	cplus_crit_sect_exit(slog->message_sect);
	return;
}

static void * syslog_initialize_object(const char * custom_program_name)
{
	struct syslog * slog = CPLUS_NULL;

	if ((slog = (struct syslog *)cplus_malloc(sizeof(struct syslog))))
	{
		CPLUS_INITIALIZE_STRUCT_POINTER(slog);

		slog->type = OBJ_TYPE;
		slog->debug_level = atoi(DEFAULT_DEBUG_LEVEL);
		slog->logger_level = atoi(DEFAULT_LOGGER_LEVEL);
		slog->enable_color = DEFAULT_ENABEL_COLOR;
		slog->message_buffer_pool = CPLUS_NULL;
		slog->message_logger_wroker = CPLUS_NULL;
		slog->message_list = CPLUS_NULL;
		slog->message_sect = CPLUS_NULL;

		if (0 > cplus_str_printf(
			slog->program_name
			, MAX_PROGRAM_NAME_SIZE + 1
			, "%s"
			, (CPLUS_NULL == custom_program_name)? program_invocation_short_name: custom_program_name))
		{
			goto exit;
		}

		if (0 > cplus_str_printf(
			slog->debug_level_envname
			, MAX_DEBUG_LEVEL_ENVNAME_SIZE + 1
			, debug_level_envname_format, slog->program_name))
		{
			goto exit;
		}

		if (0 > cplus_str_printf(
			slog->logger_level_envname
			, MAX_DEBUG_LEVEL_ENVNAME_SIZE + 1
			, logger_level_envname_format
			, slog->program_name))
		{
			goto exit;
		}

		if (CPLUS_NULL == (slog->message_sect = cplus_mutex_new()))
		{
			goto exit;
		}

		// if (0 != setenv(slog->debug_level_envname, DEFAULT_DEBUG_LEVEL, false))
		// {
		//	 goto exit;
		// }

		// if (0 != setenv(slog->logger_level_envname, DEFAULT_LOGGER_LEVEL, false))
		// {
		//	 goto exit;
		// }

		if (CPLUS_NULL != getenv(slog->debug_level_envname))
		{
			slog->debug_level = atoi(getenv(slog->debug_level_envname));
		}

		if (CPLUS_NULL != getenv(slog->logger_level_envname))
		{
			slog->logger_level = atoi(getenv(slog->logger_level_envname));
		}

		if (CPLUS_SYSLOG_LEVEL_NONE != slog->logger_level)
		{
#if SUPPORT_SYSLOG
			if (CPLUS_SYSLOG_LEVEL_INFO >= slog->logger_level)
			{ // syslog
				openlog(slog->program_name, 0, LOG_USER);

				slog->message_buffer_pool = cplus_mempool_new(
					10
					, MAX_MESSAGE_BUFFER_SIZE + 1);
				if (CPLUS_NULL == slog->message_buffer_pool)
				{
					goto exit;
				}
			}
			else
#endif
			{ // custom
				if (0 > cplus_str_printf(
					slog->file_folder_path
					, MAX_FILE_FOLDER_PATH_SIZE + 1
					, "%s"
					, DEFAULT_FILE_STORE_LOCATION))
				{
					goto exit;
				}
				else
				{
					if ((0 != mkdir(slog->file_folder_path, 0755)) && (EEXIST != errno))
					{
						goto exit;
					}
				}

				slog->message_buffer_pool = cplus_mempool_new(
					MAX_MESSAGE_COUNT
					, MAX_MESSAGE_BUFFER_SIZE + 1);
				if (CPLUS_NULL == slog->message_buffer_pool)
				{
					goto exit;
				}

				slog->message_list = cplus_llist_prev_new(MAX_MESSAGE_COUNT);
				if (CPLUS_NULL == slog->message_list)
				{
					goto exit;
				}

				slog->message_logger_wroker = cplus_task_new(
					custom_logger_executer
					, slog
					, CPLUS_NULL
					, 1000);
				if (CPLUS_NULL == slog->message_logger_wroker)
				{
					goto exit;
				}
				cplus_task_start(slog->message_logger_wroker, 0);
				cplus_task_wait_start(slog->message_logger_wroker, CPLUS_INFINITE_TIMEOUT);
			}
		}
	}
	return slog;
exit:
	cplus_syslog_delete(slog);
	return CPLUS_NULL;
}

cplus_syslog cplus_syslog_new(void)
{
	return syslog_initialize_object(CPLUS_NULL);
}

cplus_syslog cplus_syslog_new_ex(const char * custom_program_name)
{
	return syslog_initialize_object(custom_program_name);
}

bool cplus_syslog_check(cplus_object obj)
{
	return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

#if SUPPORT_SYSLOG
static int32_t to_syslog_prio(CPLUS_DEBUG_LEVEL debug_lev)
{
	switch (debug_lev)
	{
	case CPLUS_SYSLOG_LEVEL_FATAL:
		return LOG_CRIT;
	case CPLUS_SYSLOG_LEVEL_ERROR:
		return LOG_ERR;
	case CPLUS_SYSLOG_LEVEL_WARN:
		return LOG_WARNING;
	case CPLUS_SYSLOG_LEVEL_INFO:
		return LOG_INFO;
	case CPLUS_SYSLOG_LEVEL_DEBUG:
	case CPLUS_SYSLOG_LEVEL_TRACE:
		return LOG_DEBUG;
	case CPLUS_SYSLOG_LEVEL_NONE:
	default:
		errno = ENOTSUP;
		return CPLUS_FAIL;
	}
}
#endif

static int32_t syslog_logger_print(
	struct syslog * slog,
	CPLUS_DEBUG_LEVEL debug_lev,
	char * msg)
{
	int32_t len = 0;
	char * msg_buf = CPLUS_NULL;
#if SUPPORT_SYSLOG
	if (CPLUS_SYSLOG_LEVEL_INFO >= slog->logger_level)
	{
		syslog(to_syslog_prio(debug_lev), "%s", msg);
		len = strlen(msg);
	}
	else
#endif
	{
		if(MAX_MESSAGE_COUNT > cplus_llist_get_size(slog->message_list))
		{
			if ((msg_buf = (char *)cplus_mempool_alloc(slog->message_buffer_pool)))
			{
				cplus_systime stime;
				cplus_systime_get_local_time(&stime);

				len += cplus_str_printf(
					msg_buf + len
					, MAX_MESSAGE_BUFFER_SIZE - len
					, LOG_TIMESTAMP_FORMAT
					, month_strings[stime.month - 1]
					, stime.day_of_month
					, stime.hours
					, stime.minutes
					, stime.second
					, stime.millisecond
					, stime.microsecond
					, debug_level_strings[debug_lev]);
				len += cplus_str_printf(
					msg_buf + len
					, MAX_MESSAGE_BUFFER_SIZE - len
					, "%s"
					, msg);
				cplus_llist_push_front(slog->message_list, msg_buf);
			}
		}
	}
	return len;
}

#define DEBUG_PRINT(slog, debug_lev) \
	if (CPLUS_SYSLOG_LEVEL_NONE == slog->debug_level AND CPLUS_SYSLOG_LEVEL_NONE == slog->logger_level) \
	{ \
		errno = ENOTSUP; return -1; \
	} \
	do { \
		char * msg_buf = CPLUS_NULL; \
		va_list args; va_start(args, format); \
		cplus_crit_sect_enter(slog->message_sect); \
		{ \
			if (debug_lev <= slog->logger_level) \
			{ \
				msg_buf = (char *)cplus_mempool_alloc(slog->message_buffer_pool); \
				if (msg_buf) \
				{ \
					vsnprintf(msg_buf, MAX_MESSAGE_BUFFER_SIZE, format, args); \
					syslog_logger_print(slog, debug_lev, msg_buf); \
				} \
			} \
			if (debug_lev <= slog->debug_level) \
			{ \
				cplus_systime st; cplus_systime_get_local_time(&st); \
				fprintf( \
					stdout \
					, (slog->enable_color)? DEBUD_COLOR_MSG_FORMAT: DEBUD_MSG_FORMAT \
					, st.hours \
					, st.minutes \
					, st.second \
					, st.millisecond \
					, st.microsecond \
					, slog->program_name \
					, (slog->enable_color)? debug_level_color_strings[debug_lev]: debug_level_strings[debug_lev]); \
				if (msg_buf) { fprintf(stdout, "%s", msg_buf); } else { vfprintf(stdout, format, args); } \
				append_new_line(format); fflush(stdout); \
			} \
			if (msg_buf) { cplus_mempool_free(slog->message_buffer_pool, msg_buf); } \
		} \
		cplus_crit_sect_exit(slog->message_sect); \
		va_end(args); \
	} while (0)

int32_t cplus_syslog_fatal(cplus_syslog obj, const char * format, ...)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	DEBUG_PRINT(slog, CPLUS_SYSLOG_LEVEL_FATAL);
	return CPLUS_SUCCESS;
}

int32_t cplus_syslog_error(cplus_syslog obj, const char * format, ...)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	DEBUG_PRINT(slog, CPLUS_SYSLOG_LEVEL_ERROR);
	return CPLUS_SUCCESS;
}

int32_t cplus_syslog_warn(cplus_syslog obj, const char * format, ...)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	DEBUG_PRINT(slog, CPLUS_SYSLOG_LEVEL_WARN);
	return CPLUS_SUCCESS;
}

int32_t cplus_syslog_info(cplus_syslog obj, const char * format, ...)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	DEBUG_PRINT(slog, CPLUS_SYSLOG_LEVEL_INFO);
	return CPLUS_SUCCESS;
}

int32_t cplus_syslog_debug(cplus_syslog obj, const char * format, ...)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	DEBUG_PRINT(slog, CPLUS_SYSLOG_LEVEL_DEBUG);
	return CPLUS_SUCCESS;
}

int32_t cplus_syslog_trace(cplus_syslog obj, const char * format, ...)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	DEBUG_PRINT(slog, CPLUS_SYSLOG_LEVEL_TRACE);
	return CPLUS_SUCCESS;
}

CPLUS_DEBUG_LEVEL cplus_syslog_get_debug_level(cplus_syslog obj)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	return (CPLUS_DEBUG_LEVEL)slog->debug_level;
}

CPLUS_DEBUG_LEVEL cplus_syslog_get_logger_level(cplus_syslog obj)
{
	struct syslog * slog = (struct syslog *)(obj);
	CHECK_OBJECT_TYPE(obj);

	return (CPLUS_DEBUG_LEVEL)slog->logger_level;
}

#ifdef __CPLUS_UNITTEST__
#include "cplus_systime.h"

CPLUS_UNIT_TEST(sys_log, functionity)
{
	cplus_syslog slog = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (slog = cplus_syslog_new())));
	cplus_syslog_fatal(slog, "This is %s", "fatal");
	cplus_syslog_error(slog, "This is %s\n", "error");
	cplus_syslog_warn(slog, "This is %s\n", "warni");
	cplus_syslog_info(slog, "This is %s\n", "infor");
	cplus_syslog_debug(slog, "This is %s\n", "debug");
	cplus_syslog_trace(slog, "This is %s\n", "trace");
	cplus_systime_sleep_msec(100);
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_syslog_delete(slog));
	cplus_systime_sleep_msec(10);
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_syslog(void)
{
	UNITTEST_ADD_TESTCASE(sys_log, functionity);
}
#endif //__CPLUS_UNITTEST__
