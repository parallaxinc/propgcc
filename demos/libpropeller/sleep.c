#include "cog.h"

void
sleep(unsigned int n)
{
  unsigned waitcycles = _CNT + n*_clkfreq;
  __builtin_waitcnt(waitcycles, 0);
}
