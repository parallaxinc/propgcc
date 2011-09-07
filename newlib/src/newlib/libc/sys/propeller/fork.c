#include <reent.h>
#include <errno.h>

int
_fork_r(struct _reent *reent)
{
    reent->_errno = ENOSYS;
    return -1;
}
