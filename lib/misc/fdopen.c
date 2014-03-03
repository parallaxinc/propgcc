#include <stdio.h>
#include <errno.h>

/* dummy fdopen, just fails for now */
/* the real implementation would basically have to
 * duplicate the FILE structure in __files[fd];
 * the tricky part there is that we need to tell the
 * specific device driver associated with the file
 * about the duplication, otherwise its internal
 * state could get messed up
 */
FILE *fdopen(int fd, const char *mode)
{
  if (fd < 0 || fd >= FOPEN_MAX)
    {
      errno = EINVAL;
      return NULL;
    }

  errno = ENOSYS;
  return NULL;
}
