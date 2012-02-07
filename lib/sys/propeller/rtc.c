/*
 * default real time clock for propeller
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

//#define DEBUG

#include <sys/rtc.h>
#include <sys/thread.h>
#include <propeller.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/*
 * the default time is run off of the internal clock frequency
 * we keep track of current ticks in "curticks", and the base
 * (when time was set) in "baseticks"
 */

HUBDATA union {
  unsigned long long curticks;
  struct {
    unsigned int lo;
    unsigned int hi;
  } s;
} _default_ticks;

HUBDATA _atomic_t _default_ticks_lock;

HUBDATA int _default_ticks_updated = 0;

static unsigned long long baseticks;
static time_t basetime;

void
_default_update_ticks(void)
{
  unsigned int lastlo, now;

  /* update the "curticks" variable based on the current clock counter */
  /* note that this works only if we are called often enough to notice
     counter overflow, which happens about every 54 seconds or so
  */
  now = _CNT;
  __lock(&_default_ticks_lock);
  {
      lastlo = _default_ticks.s.lo;
      if (lastlo > now)
        {
          /* overflowed */
          _default_ticks.s.hi++;
        }
      _default_ticks.s.lo = now;
  }
  __unlock(&_default_ticks_lock);
}

int
_default_rtc_gettime(struct timeval *tv)
{
  unsigned long long t;
  unsigned long long rem;
  if (!_default_ticks_updated)
      _default_update_ticks();

  unsigned long long diff;
  __lock(&_default_ticks_lock);
  {
      diff  = _default_ticks.curticks - baseticks;
  }
  __unlock(&_default_ticks_lock);

  t = diff / _clkfreq;
  rem = diff % _clkfreq;

#ifdef DEBUG
  printf("curticks = %lld baseticks = %lld basetime=%lu diff = %llu t = %llu rem = %llu\n",
	 _default_ticks.curticks, baseticks, basetime, diff, t, rem);
#endif
  tv->tv_sec = basetime + (time_t)t;
  tv->tv_usec = rem * 1000000 / _clkfreq;
  return 0;
}

int
_default_rtc_settime(const struct timeval *tv)
{
  if (!_default_ticks_updated)
      _default_update_ticks();

  __lock(&_default_ticks_lock);
  {
      baseticks = _default_ticks.curticks;
  }
  __unlock(&_default_ticks_lock);

  basetime = tv->tv_sec;
  /* FIXME? the tv_usec field of tv is ignored */
  return 0;
}

/*
 * variables the main code uses to retrieve the real time
 */
int (*_rtc_gettime)(struct timeval *) = _default_rtc_gettime;
int (*_rtc_settime)(const struct timeval *) = _default_rtc_settime;
