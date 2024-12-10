#include "internal.h"

TEST(sys_log, functionity)
{
	cplus_syslog slog = CPLUS_NULL;
	EXPECT_EQ(true, (CPLUS_NULL != (slog = cplus_syslog_new())));
	cplus_syslog_fatal(slog, "This is %s", "fatal");
	cplus_syslog_error(slog, "This is %s\n", "error");
	cplus_syslog_warn(slog, "This is %s\n", "warni");
	cplus_syslog_info(slog, "This is %s\n", "infor");
	cplus_syslog_debug(slog, "This is %s\n", "debug");
	cplus_syslog_trace(slog, "This is %s\n", "trace");
	cplus_systime_sleep_msec(100);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_syslog_delete(slog));
	cplus_systime_sleep_msec(10);
	EXPECT_EQ(0, cplus_mgr_report());
}