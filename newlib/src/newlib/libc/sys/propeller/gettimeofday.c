#include <reent.h>
#include <sys/time.h>
#include <errno.h>

/* get count of seconds and milliseconds elapsed */

int 
_gettimeofday_r(struct _reent *reent, struct timeval *tv, void *tz)
{
    reent->_errno = EINVAL;
    return -1;
}

