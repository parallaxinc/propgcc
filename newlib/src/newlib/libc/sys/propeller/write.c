#include <errno.h>
#include <reent.h>

_ssize_t _write_r(struct _reent *reent, int fd, const void *buf, size_t bytes)
{
  reent->_errno = EBADF;
  return -1;
}

