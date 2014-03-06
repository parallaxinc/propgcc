#include <reent.h>
#include <errno.h>
#include <sys/stat.h>

int _fstat_r(struct _reent *reent, int fd, struct stat *buf)
{
  reent->_errno = ENOENT;
  return -1;
}

