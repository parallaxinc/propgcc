#include <reent.h>
#include <unistd.h>
#include <errno.h>

off_t _lseek_r(struct _reent *reent, int fd, off_t offset, int whence)
{
  reent->_errno = EINVAL;
  return -1;
}

