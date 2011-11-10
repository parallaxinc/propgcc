#include <time.h>
#include <sys/thread.h>
#include "cog.h"

static unsigned
waitforcnt(unsigned newval, unsigned delta)
{
#if 0
      return __builtin_propeller_waitcnt(newval, delta);
#else
      /* give up CPU time to other threads */
      while ((int)(_CNT - newval) < 0)
	(*__yield_ptr)();
      return newval + delta;
#endif
}

void
sleep(unsigned int n)
{
  unsigned waitcycles;
  unsigned second = _clkfreq;

  waitcycles = _CNT + second;
  while (n > 0) 
    {
      waitcycles = waitforcnt(waitcycles, second);
      --n;
    }
}
