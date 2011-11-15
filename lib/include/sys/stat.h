#ifndef _SYS_STAT_H
#define _SYS_STAT_H

int mkdir(const char *path, int mode);

#define S_WUSR   0400
#define S_XUSR   0200

#define S_IWRITE S_IWUSR
#define S_IEXEC  S_IXUSR

#endif
