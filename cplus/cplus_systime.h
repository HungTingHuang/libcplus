#ifndef __CPLUS_SYSTIME_H__
#define __CPLUS_SYSTIME_H__
#include <sys/time.h>
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct systime {
    uint32_t day_of_year;    // range 1 ~ 365
    uint32_t day_of_week;    // range 0 ~ 6
    uint32_t year;
    uint32_t month;          // range 1 ~ 12
    uint32_t day_of_month;   // range 1 ~ 31
    uint32_t hours;          // range 0 ~ 23
    uint32_t minutes;        // range 0 ~ 59
    uint32_t second;         // range 0 ~ 59
    uint32_t millisecond;    // range 0 ~ 999
    uint32_t microsecond;    // range 0 ~ 999
} cplus_systime;

int32_t cplus_systime_get_abstime_after_msec(struct timespec * abstime, uint32_t msec);
int32_t cplus_systime_get_abstick_after_msec(struct timespec * abstick, uint32_t msec);
uint32_t cplus_systime_get_tick(void);
uint32_t cplus_systime_diff_tick(uint32_t early_msec, uint32_t late_msec);
uint32_t cplus_systime_elapsed_tick(uint32_t msec);
int32_t cplus_systime_compare_tick(uint32_t msec1, uint32_t msec2);
void cplus_systime_sleep_msec(uint32_t msec);
int32_t cplus_systime_get_local_time(cplus_systime * stime);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_SYSTIME_H__