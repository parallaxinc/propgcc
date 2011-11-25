#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H

typedef long off_t;

int write(int fd, const void *buf, int count);
int read(int fd, void *buf, int count);
off_t lseek(int fd, off_t offset, int whence);
unsigned int sleep(unsigned int seconds);

char *getcwd(char *buf, int size);
int chdir(const char *path);
int rmdir(const char *path);

#endif
