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
#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/*
 * the default time is run off of the internal clock frequency
 * we keep track of current ticks in "curticks", and the base
 * (when time was set) in "baseticks"
 */

HUBDATA uint64_t _default_ticks;

HUBDATA _atomic_t _default_ticks_lock;

HUBDATA int _default_ticks_updated = 0;

static unsigned long long baseticks;
static time_t basetime;

void
_default_update_ticks(void)
{
  unsigned int now;
  union {
    unsigned long long curticks;
    struct {
      unsigned int lo;
      unsigned int hi;
    } s;
  } ticks;


  /* update the "_default_ticks" variable based on the current clock counter */
  /* note that this works only if we are called often enough to notice
     counter overflow, which happens about every 54 seconds or so
  */
  ticks.curticks = _getAtomic64(&_default_ticks);
  now = getcnt();

  if (ticks.s.lo > now)
    {
      /* overflowed */
      ticks.s.hi++;
    }
  ticks.s.lo = now;
  _putAtomic64(ticks.curticks, &_default_ticks);
}

int
_default_rtc_gettime(struct timeval *tv)
{
  unsigned long long t;
  unsigned long long rem;
  if (!_default_ticks_updated)
      _default_update_ticks();

  unsigned long long diff;
  diff = _getAtomic64(&_default_ticks);
  diff  = diff - baseticks;

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

  baseticks = _getAtomic64(&_default_ticks);
  basetime = tv->tv_sec;
  /* FIXME? the tv_usec field of tv is ignored */
  return 0;
}

/*
 * variables the main code uses to retrieve the real time
 */
int (*_rtc_gettime)(struct timeval *) = _default_rtc_gettime;
int (*_rtc_settime)(const struct timeval *) = _default_rtc_settime;
