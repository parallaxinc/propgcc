#include <errno.h>
#include <reent.h>

_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t bytes)
{
  reent->_errno = EBADF;
  return -1;
}

