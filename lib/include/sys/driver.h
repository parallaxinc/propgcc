#ifndef _SYS_DRIVER_H
#define _SYS_DRIVER_H

/* NOTE: this structure is included by <stdio.h>, so we cannot
 * pollute the namespace with definitions; use underscores in
 * front of names
 */
typedef struct __driver _Driver;

/*
 * generic driver for devices
 * used by stdio
 */
struct __driver {
  /* file prefix for fopen */
  const char *prefix;

  /* hook for opening/closing files */
  /* returns 0 on success, nonzero on failure */
  int (*fopen)(FILE *fp, const char *name, const char *mode);
  int (*fclose)(FILE *fp);

  /* flush pending data to disk; may be NULL if driver does not need this */
  int (*flush)(FILE *fp);

  /* single byte I/O; this may actually buffer the data */
  /* putbyte should return the byte transmitted, or EOF on error */
  int (*getbyte)(FILE *fp);
  int (*putbyte)(int c, FILE *fp);

  /* seek to a position in the file */
  int (*seek)(FILE *fp, long offset, int whence);

  /* hook for deleting files */
  int (*remove)(const char *name);

};

extern _Driver *_driverlist[];

#endif
