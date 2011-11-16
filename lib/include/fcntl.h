#ifndef _FCNTL_H
#define _FCNTL_H

/* dummy fcntl.h, not really useful */

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  4
#define O_TRUNC  8
#define O_EXCL   16

#define FD_CLOEXEC 0x100
#define F_SETFD    0x200

#endif
