#include <unistd.h>
#include <errno.h>

int access(const char *pathname, int mode){
  errno = ENOSYS;
  return -1;
}
