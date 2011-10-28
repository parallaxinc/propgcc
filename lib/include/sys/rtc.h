/*
 * real time clock support
 */

#ifndef _SYS_RTC_H
#define _SYS_RTC_H

#include <sys/time.h>

int (*_rtc_gettime)(struct timeval *tv);
int (*_rtc_settime)(const struct timeval *tv);

#endif
