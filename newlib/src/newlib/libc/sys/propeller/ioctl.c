#include <stdarg.h>
#include <errno.h>

int ioctl(int fd, int request, ...){
   errno = EINVAL;
   return -1;
}

