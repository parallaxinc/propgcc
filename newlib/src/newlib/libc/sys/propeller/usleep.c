/* usleep: sleep for (at least) the specified number of microseconds */
/* returns 0 for success, -1 for failure */

#include <unistd.h>
#include <errno.h>

int
usleep(useconds_t usecs)
{
  errno = EINTR;
  return -1;
}
