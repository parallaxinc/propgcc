#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int creat(const char *name, mode_t mode){
  return open(name, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

