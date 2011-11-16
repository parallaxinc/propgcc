#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H

int write(int fd, const void *buf, int count);
int read(int fd, const void *buf, int count);
unsigned int sleep(unsigned int seconds);

char *getcwd(char *buf, int size);
int chdir(const char *path);
int rmdir(const char *path);

#endif
