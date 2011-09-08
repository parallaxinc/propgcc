#include <errno.h>
#include <reent.h>

int _close_r(struct _reent *reent, int fd)
{
  reent->_errno = EIO;
  return -1;
}

