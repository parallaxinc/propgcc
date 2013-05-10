/*
 * implementation of close()
 * written by Eric R. Smith and placed in the public domain
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int close(int fd)
{
  FILE *fp;

  if (fd < 0 || fd >= FOPEN_MAX || !(fp = &__files[fd]))
    {
      errno = EINVAL;
      return -1;
    }

  return fclose(fp);
}
