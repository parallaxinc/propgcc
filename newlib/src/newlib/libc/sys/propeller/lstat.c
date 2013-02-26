#include <errno.h>
#include <sys/stat.h>

int lstat(const char *file_name, struct stat *buf){
  errno = ENOENT;
  return -1;
}

