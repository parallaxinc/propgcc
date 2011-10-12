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

time_t
_default_rtc_gettime(void)
{
  unsigned long long t;

  update_ticks();
  t = (ticku.curticks - baseticks) / _clkfreq;
#ifdef DEBUG
  printf("curticks = %lld baseticks = %lld basetime=%lu t = %llu\n",
	 ticku.curticks, baseticks, basetime, t);
#endif
  return basetime + (time_t)t;
}

void
_default_rtc_settime(time_t t)
{
  update_ticks();
  baseticks = ticku.curticks;
  basetime = t;
}

/*
 * variables the main code uses to retrieve the real time
 */
time_t (*_rtc_gettime)(void) = _default_rtc_gettime;
void (*_rtc_settime)(time_t) = _default_rtc_settime;
