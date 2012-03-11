#include <errno.h>
#include <reent.h>

int _kill_r(struct _reent *reent, int pid, int sig)
{
    reent->_errno = EINVAL;
    return -1;
}

