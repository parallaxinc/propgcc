#include <reent.h>
#include <errno.h>

int _link_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
  reent->_errno = EACCES;
  return -1;
}

