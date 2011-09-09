#include <reent.h>
#include <errno.h>

int
_execve_r(struct _reent *reent, char *name, char **argv, char **env)
{
    reent->_errno = ENOSYS;
    return -1;
}
