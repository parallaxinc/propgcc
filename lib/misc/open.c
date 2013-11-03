#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/*
 * the "mode" parameter is optional, and is in fact
 * unused in our port (it signifies the file permissions to
 * give to the file if it is created)
 */
int
open(const char *pathname, int flags, int mode)
{
  char modestr[4];
  FILE *f;

  memset(modestr, 0, sizeof(modestr));

  switch (flags & O_ACCMODE) {
  case O_RDONLY:
    modestr[0] = 'r';
    break;
  case O_WRONLY:
    modestr[0] = (flags & O_APPEND) ? 'a' : 'w';
    break;
  default:
    modestr[1] = '+';
    if (flags & O_APPEND)
      modestr[0] = 'a';
    else if (flags & (O_CREAT|O_TRUNC))
      modestr[0] = 'w';
    else
      modestr[0] = 'r';
    break;
  }
  f = fopen(pathname, modestr);
  if (!f) {
    return -1;
  }
  return fileno(f);
}
