#include <reent.h>
#include <errno.h>

int
_execve_r(struct _reent *reent, const char *name, char * const *argv, char * const *env)
{
    reent->_errno = ENOSYS;
    return -1;
}
