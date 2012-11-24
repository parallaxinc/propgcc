#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

int stat(const char *path, struct stat *buf)
{
  int fd = open(path, O_RDONLY);
  int r;

  if (fd < 0) return fd;
  r = fstat(fd, buf);
  close(fd);
  return r;
}

/*
 * for now make this a dummy
 * we could actually use seek to fill in the file size, and a few
 * other fields may be derivable from the driver
 */
int fstat(int fd, struct stat *buf)
{
  FILE *fp;

  if (fd < 0 || fd >= FOPEN_MAX)
    {
      errno = EINVAL;
      return -1;
    }

  fp = &__files[fd];
  if (!fp || !fp->_drv)
    {
      errno = EINVAL;
      return -1;
    }

  /* here's where we should use the driver to fill in the stat buffer,
     if we want to do this right!
     for now, just return a "not supported" error
  */
  errno = ENOSYS;
  return -1;
}
