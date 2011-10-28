/*
 * default real time clock for propeller
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

//#define DEBUG

#include <sys/rtc.h>
#include <cog.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/*
 * the default time is run off of the internal clock frequency
 * we keep track of current ticks in "curticks", and the base
 * (when time was set) in "baseticks"
 */

static union {
  unsigned long long curticks;
  struct {
    unsigned int lo;
    unsigned int hi;
  } s;
} ticku;

static unsigned long long baseticks;
static time_t basetime;

static void
update_ticks(void)
{
  unsigned int lastlo, now;

  /* update the "curticks" variable based on the current clock counter */
  /* note that this works only if we are called often enough to notice
     counter overflow, which happens about every 54 seconds or so
  */
  now = _CNT;
  lastlo = ticku.s.lo;
  if (lastlo > now)
    {
      /* overflowed */
      ticku.s.hi++;
    }
  ticku.s.lo = now;
}

int
_default_rtc_gettime(struct timeval *tv)
{
  unsigned long long t;
  unsigned long long rem;

  update_ticks();
  t = (ticku.curticks - baseticks) / _clkfreq;
  rem = (ticku.curticks - baseticks) % _clkfreq;

#ifdef DEBUG
  printf("curticks = %lld baseticks = %lld basetime=%lu t = %llu\n",
	 ticku.curticks, baseticks, basetime, t);
#endif
  tv->tv_sec = basetime + (time_t)t;
  tv->tv_usec = rem * 1000000 / _clkfreq;
  return 0;
}

int
_default_rtc_settime(const struct timeval *tv)
{
  update_ticks();
  baseticks = ticku.curticks;
  basetime = tv->tv_sec;
  /* FIXME? the tv_usec field of tv is ignored */
  return 0;
}

/*
 * variables the main code uses to retrieve the real time
 */
int (*_rtc_gettime)(struct timeval *) = _default_rtc_gettime;
int (*_rtc_settime)(const struct timeval *) = _default_rtc_settime;
