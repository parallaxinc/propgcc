#include <time.h>
#include <sys/thread.h>
#include <unistd.h>
#include "cog.h"

int
usleep(unsigned int n)
{
  unsigned waitcycles;
  unsigned usecond = _clkfreq/1000000;

  waitcycles = _CNT + n*usecond;
  __napuntil(waitcycles);

  return 0;
}
