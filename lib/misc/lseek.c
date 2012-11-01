#include <stdio.h>
#include <unistd.h>
#include <errno.h>

off_t
lseek(int fd, off_t offset, int whence)
{
  FILE *fp;

  if (fd < 0 || fd >= FOPEN_MAX)
    {
      errno = EINVAL;
      return -1;
    }

  fp = &__files[fd];
  return fseek(fp, offset, whence);
}
