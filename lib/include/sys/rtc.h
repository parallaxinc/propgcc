/*
 * real time clock support
 */

#ifndef _SYS_RTC_H
#define _SYS_RTC_H

#include <time.h>

time_t (*_rtc_gettime)(void);
void (*_rtc_settime)(time_t);

#endif
