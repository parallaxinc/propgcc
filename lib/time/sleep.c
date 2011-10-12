#include <time.h>
#include "cog.h"

void
sleep(unsigned int n)
{
  unsigned waitcycles;
  unsigned second = _clkfreq;

  waitcycles = _CNT + second;
  while (n > 0) 
    {
      waitcycles = __builtin_propeller_waitcnt(waitcycles, second);
      --n;
    }
}
