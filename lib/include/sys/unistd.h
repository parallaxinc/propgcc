#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H

#if defined(__cplusplus)
extern "C" {
#endif

  typedef long off_t;
  typedef unsigned int useconds_t;

  int write(int fd, const void *buf, int count);
  int read(int fd, void *buf, int count);
  off_t lseek(int fd, off_t offset, int whence);
  unsigned int sleep(unsigned int seconds);

  char *getcwd(char *buf, int size);
  int chdir(const char *path);
  int rmdir(const char *path);

  int usleep(useconds_t usec);

#if defined(__cplusplus)
}
#endif

#endif
