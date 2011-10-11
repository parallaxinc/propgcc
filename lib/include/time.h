#ifndef _TIME_H
#define _TIME_H

typedef unsigned int clock_t;
extern clock_t _clkfreq;
/* the actual frequency the machine is running at may vary */
#define CLOCKS_PER_SEC _clkfreq

/* our time_t is the same as the Posix time_t:
 *  86400 * (number of days past the epoch) + (seconds elapsed today)
 * where "the epoch" is 00:00:00 UTC Jan. 1, 1970.
 *
 * this is often misquoted as "seconds elapsed since the epoch", but in
 * fact it is not: it ignores the existence of leap seconds
 */
typedef unsigned long time_t;

#ifndef _STRUCT_TM_DEFINED
#define _STRUCT_TM_DEFINED
/* time representing broken down calendar time */
struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year; /* years since 1900 */
  int tm_wday; /* days since Sunday */
  int tm_yday; /* days since January 1 */
  int tm_isdst; /* if > 0, DST is in effect, if < 0 info is not known */
};
#endif

clock_t clock(void);
time_t  time(time_t *);
double  difftime(time_t time2, time_t time1);

time_t mktime(struct tm *stm);

struct tm *_gmtime_r(const time_t *t, struct tm *stm);
struct tm *gmtime(const time_t *, struct tm *);
struct tm *_localtime_r(const time_t *, struct tm *);
struct tm *localtime(const time_t *);

__SIZE_TYPE__ strftime(char *s, __SIZE_TYPE__ max, const char *format, const struct tm *tm);

#endif
