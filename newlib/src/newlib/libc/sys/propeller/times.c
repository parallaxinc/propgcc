#include <reent.h>
#include <sys/times.h>

/* get count of seconds and milliseconds elapsed */

clock_t
_times_r(struct _reent *reent, struct tms *buf)
{
    /* return a count of milliseconds */
  return 0;
}

