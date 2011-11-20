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

  /* optional hooks for single character I/O */
  /* these are only needed to use the generic terminal driver
     for read and write */
  int (*getbyte)(FILE *fp);
  int (*putbyte)(int c, FILE *fp);
};

extern _Driver *_driverlist[];

/* some standard functions that are useful in drivers */
int _null_read(FILE *fp, unsigned char *buf, int size);
int _null_write(FILE *fp, unsigned char *buf, int size);

/* use the putbyte and getbyte functions to do cooked I/O */
int _term_read(FILE *fp, unsigned char *buf, int size);
int _term_write(FILE *fp, unsigned char *buf, int size);

#endif
