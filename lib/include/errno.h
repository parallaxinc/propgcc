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
#define EFAULT    20    /* Bad address */
#define EPIPE     21    /* Broken pipe */
#define EBUSY     22    /* Device or resource busy */
#define EXDEV     23    /* Cross device link */
#define ENOSPC    24    /* No space on device */
#define EINTR     25    /* System call interrupted */
#define ENODEV    26
#define ENFILE    27
#define EDEADLK   28

  /* unlikely errors */
#define ENOBUFS      50
#define ECHILD       51
#define ENOLCK       52
#define ESRCH        53
#define EMLINK       54
#define ELOOP        55
#define EPROTOTYPE   56
#define ENXIO        57

  /* networking errors */
#define EAFNOSUPPORT 100
#define EADDRINUSE   101
#define EADDRNOTAVAIL 102
#define EISCONN      103
#define E2BIG        104
#define ECONNREFUSED 105
#define ECONNRESET   106
#define EDESTADDRREQ 107
#define EHOSTUNREACH 108
#define EMSGSIZE     109
#define ENOMSG       110
#define ENOPROTOOPT  111
#define ENETDOWN     112
#define ENETRESET    113
#define ENETUNREACH  114
#define ENOTSOCK     115
#define EINPROGRESS  116
#define EPROTONOSUPPORT 117
#define ENOTCONN     118
#define ECONNABORTED 119
#define EALREADY     120
#define ETIMEDOUT    121

  /* some aliases */
#define EWOULDBLOCK EAGAIN
#define ESPIPE       EPIPE
#define ENOEXEC      ENOSYS
#define EFBIG        ERANGE
#define EOPNOTSUPP   ENOSYS
#define ENOTTY       ENOSEEK
#define EPERM        EACCES

#if defined(__cplusplus)
}
#endif

#endif
