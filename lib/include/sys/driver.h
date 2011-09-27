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

  /* multi byte I/O */
  int (*read)(FILE *fp, unsigned char *buf, int size);
  int (*write)(FILE *fp, unsigned char *buf, int size);

  /* seek to a position in the file */
  int (*seek)(FILE *fp, long offset, int whence);

  /* hook for deleting files */
  int (*remove)(const char *name);

};

extern _Driver *_driverlist[];

#endif
