#ifndef _ERRNO_H
#define _ERRNO_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/thread.h>

#define errno (_TLS->errno)

#define EOK      0      /* Undefined error */
#define	EDOM     1      /* Math arg out of domain of func */
#define	ERANGE   2	/* Math result not representable */
#define EILSEQ   3      /* Illegal sequence */
#define	ENOENT   4	/* No such file or directory */
#define	EBADF    5	/* Bad file number */
#define	EACCES   6	/* Permission denied */
#define	ENOMEM   7	/* Not enough core */
#define	EAGAIN   8	/* Temporary failure */
#define	EEXIST   9	/* File exists */
#define	EINVAL  10	/* Invalid argument */
#define	EMFILE  11	/* Too many open files */
#define	EIO     12	/* I/O error */
#define	ENOTDIR 13	/* Not a directory */
#define	EISDIR  14	/* Is a directory */
#define	EROFS   15	/* Read only file system */
#define ENOSYS  16	/* Function not implemented */
#define ENOTEMPTY 17	/* Directory not empty */
#define ENAMETOOLONG 18	/* File or path name too long */
#define ENOSEEK   19    /* Device not seekable */

#define ENUMERRORS 20
#define EBUSY EAGAIN

#if defined(__cplusplus)
}
#endif

#endif
