/**
 * @file include/fcntl.h
 *
 * @brief Provides fcntl() interface API macros.
 *
 * The fcntl() function is only provided in this library to support libstdc++ std namespace.
 *
 * The fcntl() function is not really useful as Propeller-GCC does not implement fcntl().
 *
 */
#ifndef _FCNTL_H
#define _FCNTL_H

/* dummy fcntl.h, not really useful as open is not implemented */

/** Set open file status to read only */
#define O_RDONLY 0

/** Set open file status to write only */
#define O_WRONLY 1

/** Set open file status to read-write */
#define O_RDWR   2

/** Set open file mode to create */
#define O_CREAT  4

/** Set open file mode to truncate */
#define O_TRUNC  8

/** Set open file mode to exclusive */
#define O_EXCL   16

/** 
 * Definition provided for convenience and libstdc++ build only.
 * Propeller-GCC does not allow forking applications.
 *
 * If bit is 0, the file descriptor will remain open across
 * execve(2), otherwise it will be closed.
 */
#define FD_CLOEXEC 0x100

/**
 * Definition provided for convenience and libstdc++ build only.
 * Propeller-GCC does not implement fcntl().
 *
 * Set the file descriptor flags to the value specified by arg.
 */
#define F_SETFD    0x200

#endif
