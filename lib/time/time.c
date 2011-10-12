#include <time.h>
#include <sys/rtc.h>

time_t
time(time_t *tp)
{
  unsigned now = (*_rtc_gettime)();
  if (tp)
    *tp = now;
  return now;
}
