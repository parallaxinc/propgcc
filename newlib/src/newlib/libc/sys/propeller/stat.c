#include <reent.h>
#include <errno.h>
#include <sys/stat.h>

int _stat_r(struct _reent *reent, const char *path, struct stat *buf)
{
  reent->_errno = EACCES;
  return -1;
}

