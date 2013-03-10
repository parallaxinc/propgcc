#include <reent.h>
#include <errno.h>

int _isatty_r ( struct _reent *reent, int fd )
{
  return 1;
}

