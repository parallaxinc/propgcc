#include <time.h>
#include "cog.h"

time_t
time(time_t *tp)
{
  unsigned now = _CNT / _clkfreq;
  if (*tp)
    *tp = now;
  return now;
}
