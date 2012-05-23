/**
 * @file include/sys/driver.h
 *
 * @brief Contains driver API for stdio devices.
 *
 * Copyright (c) 2011-2012 by Parallax, Inc.
 * All rights MIT Licensed.
 */

#ifndef _SYS_DRIVER_H
#define _SYS_DRIVER_H

/**
 * Typedef for the _Driver struct
 * 
 * @note This structure is included by <stdio.h>, so we cannot
 * pollute the namespace with definitions; use underscores in
 * front of names
 */
typedef struct __driver _Driver;

/**
 * @brief Generic driver struct for devices.
 * This struct is used by stdio.
 *
 * The purpose is to allow replacing the stdio functions.
 * Any device can be attached to stdio functions with this struct.
 */
struct __driver {
  /** file prefix for fopen */
  const char *prefix;

  /**
   * Prototype for opening files.
   * @param[in,out] fp This is the file pointer for the driver.
   * @param[in] name This is the device name string.
   * @param[in] mode This is the device open mode string.
   * @returns 0 on success, nonzero on failure
   */
  int (*fopen)(FILE *fp, const char *name, const char *mode);

  /**
   * hook for closing files
   * returns 0 on success, nonzero on failure
   */
  int (*fclose)(FILE *fp);

  /** multi byte I/O read from a device */
  int (*read)(FILE *fp, unsigned char *buf, int size);
  /** multi byte I/O write to a device */
  int (*write)(FILE *fp, unsigned char *buf, int size);

  /** seek to a position in the file */
  int (*seek)(FILE *fp, long offset, int whence);

  /** hook for deleting files */
  int (*remove)(const char *name);

  /**
   * Optional hook for single character input.
   * It is needed for using the generic terminal driver for read.
   * @param fp The file pointer.
   * @returns character read by the function.
   */
  int (*getbyte)(FILE *fp);

  /**
   * Optional hook for single character output.
   * It is needed for using the generic terminal driver for write.
   * @param c The character to write.
   * @param fp The file pointer.
   * @returns character read by the function.
   */
  int (*putbyte)(int c, FILE *fp);
};

/**
 * @brief Device driver array of __driver structs
 *
 * Typically this array can be defined by users for setting up
 * stdin, stdout, stderr FILE drivers, SD card FILE drivers, and others.
 *
 * The _Driver list is a list of all drivers we can use in the program.
 * The default _InitIO function opens stdin, stdout, and stderr based
 * on the first driver in the list (typically the serial driver).
 * The serial driver could be replaced by a TV/Keyboard driver.
 * 
 * When defined by the user, the struct may look like this:
 * @verbatim

extern _Driver _SimpleSerialDriver;
extern _Driver _FileDriver;

_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  &_FileDriver,
  NULL
};

 @endverbatim
 * The NULL driver ends the _Driver list.
 *
 */
extern _Driver *_driverlist[];

/* some standard functions that are useful in drivers */

/** Use this when no other read function is applicable */
int _null_read(FILE *fp, unsigned char *buf, int size);
/** Use this when no other write function is applicable */
int _null_write(FILE *fp, unsigned char *buf, int size);

/** Use as getbyte function to do cooked I/O input */
int _term_read(FILE *fp, unsigned char *buf, int size);
/** Use as putbyte function to do cooked I/O output */
int _term_write(FILE *fp, unsigned char *buf, int size);

#endif
