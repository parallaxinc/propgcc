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
 * This struct is used by stdio. It lets a program define custom devices.
 *
 * @see __driver for more information on custom devices.
 *
 * @note This structure is included by <stdio.h>, so we cannot
 * pollute the namespace with definitions; use underscores in
 * front of names
 */
typedef struct __driver _Driver;

/**
 * @brief Generic and customizable driver struct for stdio devices.
 *
 * @details
 * The purpose is to allow replacing the stdio functions.
 * Any device can be attached to stdio functions with this struct.
 *
 * Typically the __driver array is defined by users for setting up
 * stdin, stdout, stderr FILE drivers, SD card FILE drivers, and others.
 *
 * The _Driver list is a list of all drivers we can use in the program.
 * The default _InitIO function opens stdin, stdout, and stderr based
 * on the first driver in the list (typically the serial driver).
 * The serial driver could be replaced by a TV/Keyboard driver.
 * 
 * When defined by the user, the array of structs may look like this:
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
 * The device driver interface is the __driver struct.
 * By defining the struct in the device driver, one connects driver
 * functions to the _Driver.
 *
 * In a TV output device driver for example, we need to define
 * fopen() for the _InitIO() routine, fwrite(), and fclose().
 *
 * @verbatim

_Driver TvDriver = {
  TvPrefix,     // TvPrefix is the device driver name "TV"
  Tv_fopen,     // fopen starts the TV COG (term for Propeller core).
  Tv_fclose,    // fclose stops the TV COG if necessary, etc....
  _null_read,   // use _null_read instead of defining Tv_fread
  Tv_write,     // fwrite is used to send characters to the TV
  NULL,         // seek; not applicable
  NULL,         // remove; not applicable
  NULL,         // getbyte; not applicable
  Tv_putbyte,   // putbyte: write a single byte
};

 @endverbatim
 *
 * Of course it is not necessary to use a stdio method for TV output.
 * There are some cases where the stdio infrastructure is not necessary.
 * The standard stdio library is relatively big, but as small as possible.
 * 
 * The __driver mechanism gives the program flexibility in a standard way.
 * The types of drivers are limited only by the programmer's imagination.
 *
 * Some VGA demo programs have been written entirely in C.
 */
struct __driver {
  /**
   * The file "device" prefix for fopen.
   * @details
   * This string allows users to name their devices an pass startup
   * information to the driver if nessesary.
   *
   * Some library device names: "SSER", "FDS", "TV", etc....
   *
   * The default Propeller-GCC stdio console device name is "SSER".
   * 
   */
  const char *prefix;

  /**
   * Prototype for opening files.
   * @details A function for the user's device must be provided by the user's driver.
   * @param[out] fp This is the file pointer for the driver.
   * @param[in] name This is the device name string.
   * @param[in] mode This is the device open mode string.
   * @returns 0 on success, nonzero on failure
   */
  int (*fopen)(FILE *fp, const char *name, const char *mode);

  /**
   * Prototype for closing files to be filled in by user's driver.
   * @param[in,out] fp FILE pointer set by previous fopen() call.
   * @returns 0 on success, nonzero on failure
   */
  int (*fclose)(FILE *fp);

  /**
   * Prototype for reading multi byte I/O.
   * @details A function for the user's device must be provided by the user's driver.
   * @param[in,out] fp FILE pointer set by previous fopen() call.
   * @param[out] buf A char buffer of at least size length where data is put after read.
   * @param[in] size The size of the buf parameter.
   * @returns The number of bytes read.  If an error occurs, or the
   * end-of-file is reached, the return value is a short object count (or zero).
   */
  int (*read)(FILE *fp, unsigned char *buf, int size);

  /**
   * Prototype for writing multi byte I/O.
   * @details A function for the user's device must be provided by the user's driver.
   * @param[in,out] fp FILE pointer set by previous fopen() call.
   * @param[in] buf A char buffer of at least size length where data is put befoe write.
   * @param size The size of the buf to write.
   * @returns The number of bytes read.  If an error occurs, or the
   * end-of-file is reached, the return value is a short object count (or zero).
   */
  int (*write)(FILE *fp, unsigned char *buf, int size);

  /**
   * Prototype for seek to a position in the file.
   * @details A function for the user's device must be provided by the user's driver.
   * @param[in,out] fp FILE pointer set by previous fopen() call.
   * @param[in] offset The offset to add to the file position specified by whence.
   * @param[in] whence The start position specifier: SEEK_SET, SEEK_CUR, or SEEK_END.
   * @returns Zero on success, or -1 on error with errno set to indicate error.
   */
  int (*seek)(FILE *fp, long offset, int whence);

  /**
   * Prototype for removing a file or directory from the file system.
   * @param name The name of the file to remove.
   * @returns Zero on success, or -1 on error with errno set to indicate error.
   */
  int (*remove)(const char *name);

  /**
   * Optional prototype for single character input.
   * @details Function getbyte is needed for reading the generic terminal driver.
   * @param[in] fp The file pointer.
   * @returns character read by the function.
   */
  int (*getbyte)(FILE *fp);

  /**
   * Optional prototype for single character output.
   * @details Function putbyte is needed for writing to the generic terminal driver.
   * @param c The character to write.
   * @param[in] fp The file pointer.
   * @returns character written by the function.
   */
  int (*putbyte)(int c, FILE *fp);
};

/**
 * @brief Device driver array of __driver structs
 *
 * @see Detailed Description in include/sys/driver.h
 *
 */
extern _Driver *_driverlist[];

/* some standard functions that are useful in drivers */

/** Use this when no other read function is applicable */
int _null_read(FILE *fp, unsigned char *buf, int size);
/** Use this when no other write function is applicable */
int _null_write(FILE *fp, unsigned char *buf, int size);

/** Uses getbyte function to do cooked I/O input */
int _term_read(FILE *fp, unsigned char *buf, int size);
/** Uses putbyte function to do cooked I/O output */
int _term_write(FILE *fp, unsigned char *buf, int size);

#endif
