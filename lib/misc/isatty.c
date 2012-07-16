#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int isatty(int fd)
{
  FILE *fp;

  if (fd < 0 || fd >= FOPEN_MAX || NULL == (fp = &__files[fd]) || !fp->_drv)
    {
      errno = EINVAL;
      return -1;
    }

  return (fp->_drv->getbyte != NULL) || (fp->_drv->putbyte != NULL);
}
