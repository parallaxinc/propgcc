/* placeholder sys/types.h */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <sys/size_t.h>
#include <sys/wchar_t.h>
#include <time.h> /* for time_t */

typedef long off_t;

struct stat {
  int st_dev;  /* ID of device containing file */
  int st_ino;  /* inode number */
  unsigned int st_mode; /* protection */
  int st_nlink;
  unsigned short st_uid;
  unsigned short st_gid;
  int st_rdev;
  long st_size;
  long st_blksize;
  long st_blocks;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

#endif
