#include <time.h>
#include <sys/thread.h>
#include "cog.h"

void
sleep(unsigned int n)
{
  unsigned waitcycles;
  unsigned second = _clkfreq;

  waitcycles = _CNT;
  while (n > 0) 
    {
      waitcycles += second;
      __napuntil(waitcycles);
      --n;
    }
}
