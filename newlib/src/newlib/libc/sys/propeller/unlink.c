#include <reent.h>
#include <errno.h>

int _unlink_r(struct _reent *reent, const char *pathname)
{
  reent->_errno = ENOENT;
  return -1;
}

