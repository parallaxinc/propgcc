#include <stdio.h>
#include <unistd.h>
#include <errno.h>

static int
_doio(int fd, void *buf, int count, int isread)
{
  FILE *fp;

  if (fd < 0 || fd >= FOPEN_MAX)
    {
      errno = EINVAL;
      return -1;
    }

  fp = &__files[fd];
  if (isread)
    return (fp->_drv->read)(fp, buf, count);
  else
    return (fp->_drv->write)(fp, buf, count);
}

int
read(int fd, void *buf, int count)
{
  return _doio(fd, buf, count, 1);
}

int
write(int fd, const void *buf, int count)
{
  return _doio(fd, (void *)buf, count, 0);
}
