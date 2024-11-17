/******************************************************************
* @file: systime.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <sys/time.h>
#include <time.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_systime.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 1)

int32_t cplus_systime_get_abstime_after_msec(
    struct timespec * abstime
    , uint32_t msec)
{
    struct timeval tv = {0, 0};
    CHECK_NOT_NULL(abstime, CPLUS_FAIL);

    if (0 != gettimeofday(&tv, CPLUS_NULL))
    {
        return CPLUS_FAIL;
    }

    tv.tv_sec += (msec / 1000);
    tv.tv_usec += (msec % 1000) * 1000;
    tv.tv_sec += tv.tv_usec / (1000 * 1000);
    tv.tv_usec = tv.tv_usec % (1000 * 1000);
    ((struct timespec *)abstime)->tv_sec = tv.tv_sec;
    ((struct timespec *)abstime)->tv_nsec = tv.tv_usec * 1000;

    return CPLUS_SUCCESS;
}

int32_t cplus_systime_get_abstick_after_msec(
    struct timespec * abstick
    , uint32_t msec)
{
    struct timespec tp = {0, 0};
    CHECK_NOT_NULL(abstick, CPLUS_FAIL);

    if (0 != clock_gettime(CLOCK_MONOTONIC, &tp))
    {
        return CPLUS_FAIL;
    }

    tp.tv_sec += (msec / 1000);
    tp.tv_nsec += (msec % 1000) * (1000 * 1000);
    tp.tv_sec += tp.tv_nsec / (1000 * 1000 * 1000);
    tp.tv_nsec = tp.tv_nsec % (1000 * 1000 * 1000);
    ((struct timespec *)abstick)->tv_sec = tp.tv_sec;
    ((struct timespec *)abstick)->tv_nsec = tp.tv_nsec;

    return CPLUS_SUCCESS;
}

uint32_t cplus_systime_get_tick(void)
{
    uint32_t ms = 0;
    struct timespec tp = {0, 0};

    do
    {
        clock_gettime(CLOCK_MONOTONIC, &tp);
        ms = (tp.tv_sec * 1000);
        ms += (tp.tv_nsec / (1000 * 1000));
    }
    while(0 == ms);

    return ms;
}

uint32_t cplus_systime_diff_tick(uint32_t early_msec, uint32_t late_msec)
{
    uint32_t diff_ms = 0;

    if (0 != early_msec)
    {
        if (late_msec < early_msec)
        {
            diff_ms = UINT32_MAX - early_msec;
            diff_ms += late_msec;
            diff_ms += 1;
        }
        else
        {
            diff_ms = late_msec - early_msec;
        }
    }
    else
    {
        diff_ms = late_msec;
    }

    return diff_ms;
}

uint32_t cplus_systime_elapsed_tick(uint32_t msec)
{
    return cplus_systime_diff_tick(msec, cplus_systime_get_tick());
}

int32_t cplus_systime_compare_tick(uint32_t msec1, uint32_t msec2)
{
    if (msec1 == msec2)
    {
        return 0;
    }
    else
    {
		if (msec1 > msec2)
		{
			if ((msec1 - msec2) < (UINT32_MAX / 2))
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if ((msec2 - msec1) < (UINT32_MAX / 2))
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
    }
}

void cplus_systime_sleep_msec(uint32_t msec)
{
    uint32_t ms = 0;

    ms = ((0 == msec) ? 1 : msec);

    if (ms < 1000)
	{
		usleep(ms * 1000);
	}
	else
	{
		struct timespec req;
		struct timespec rem;

		req.tv_sec = ms / 1000;
		req.tv_nsec = ms % 1000;
		req.tv_nsec *= (1000 * 1000);

		while (1)
		{
			int32_t res = nanosleep(&req, &rem);
			if (0 == res)
			{
				break;
			}

			if (-1 == res AND EINTR == errno)
			{
				cplus_mem_cpy(&req, &rem, sizeof(req));
			}
			else
			{
				break;
			}
		}
	}
}

static int32_t convert_to_systime(cplus_systime * stime, struct tm * tm)
{
    CHECK_NOT_NULL(stime, CPLUS_FAIL);
    CHECK_NOT_NULL(tm, CPLUS_FAIL);

    stime->day_of_year  = tm->tm_yday + 1;
    stime->day_of_week  = tm->tm_wday + 1;
    stime->year         = tm->tm_year + 1900;
    stime->month        = tm->tm_mon + 1;
    stime->day_of_week  = tm->tm_wday;
    stime->day_of_month = tm->tm_mday;
    stime->hours        = tm->tm_hour;
    stime->minutes      = tm->tm_min;
    stime->second       = tm->tm_sec;

    return CPLUS_SUCCESS;
}

int32_t cplus_systime_get_local_time(cplus_systime * stime)
{
    struct tm timeinfo;
    struct timeval tv;

    CHECK_NOT_NULL(stime, CPLUS_FAIL);

    tzset();

    if (0 != gettimeofday(&tv, CPLUS_NULL))
	{
		return CPLUS_FAIL;
	}

    if (CPLUS_NULL == localtime_r(&tv.tv_sec, &timeinfo))
    {
        return CPLUS_FAIL;
    }

    stime->millisecond = (tv.tv_usec / 1000);
    stime->microsecond = (tv.tv_usec % 1000);
    return convert_to_systime(stime, &timeinfo);
}
