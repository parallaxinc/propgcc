#include <reent.h>
#include <errno.h>
#include <fcntl.h>

int _open_r(struct _reent *reent, const char *path, int access, int mode)
{
  reent->_errno = ENOENT;
  return -1;
}

