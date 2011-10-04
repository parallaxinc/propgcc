#include <stdio.h>
#include <sys/unistd.h>

int
write(int fd, const void *buf, int count)
{
  FILE *f = &__files[fd];
  return fwrite(buf, 1, count, f);
}
