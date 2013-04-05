/*
 * Implementation of stdio library functions
 * Copyright (c) 2011 Parallax, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */

#ifndef _STDIO_H
#define _STDIO_H

#if defined(__cplusplus)
extern "C" {
#endif

  /* forward declaration */
  typedef struct _FILE FILE;

#include <sys/size_t.h>
#include <sys/va_list.h>
#include <sys/driver.h>
#include <sys/null.h>
#include <sys/thread.h>

#if defined(__GNUC__)
#define _PRINTF_FUNC __attribute__((format (printf, 1, 2)))
#define _FPRINTF_FUNC __attribute__((format (printf, 2, 3)))
#else
#define _PRINTF_FUNC
#define _FPRINTF_FUNC
#endif

#define EOF       (-1)
#define BUFSIZ     512
#define FOPEN_MAX    8
#define FILENAME_MAX 256

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

  typedef unsigned long fpos_t;

  struct _FILE {

/* FILE structure flags */
#define	_IOREAD		0x0001		/* file may be read from */
#define	_IOWRT		0x0002		/* file may be written to */
#define	_IOBIN		0x0004		/* file is in "binary" mode */
#define	_IODEV		0x0008		/* file is a character device */
#define _IOCOOKED       0x0010          /* do terminal input processing */
#define	_IORW		0x0080		/* file is open for update (r+w) */
#define	_IOFBF		0x0100		/* i/o is fully buffered */
#define	_IOLBF		0x0200		/* i/o is line buffered */
#define	_IONBF		0x0400		/* i/o is not buffered */
#define	_IOFREEBUF	0x0800		/* buffer needs freeing */
#define	_IOEOF		0x1000		/* EOF has been reached */
#define	_IOERR		0x2000		/* an error has occured */
#define _IOAPPEND       0x4000          /* data should be appended to file */
#define _IONONBLOCK     0x8000          /* i/o should be non-blocking */

    long		_cnt;		/* # of bytes in buffer */
    unsigned char	*_ptr;		/* current buffer pointer */
    unsigned char	*_base;		/* base of file buffer */
    unsigned int	_flag;		/* file status flags */
    long		_bsiz;		/* buffer size */

    /* lock for multi-threaded access to FILE struct */
    _atomic_t           _lock;

    /* driver for this file */
    struct __driver *_drv;

    #define _SMALL_BUFSIZ 8
    /* a default buffer for character I/O */
    unsigned char _chbuf[_SMALL_BUFSIZ];

    /* auxiliary information that the driver may need */
    unsigned long drvarg[4];

  };

  extern FILE __files[];
#define stdin (&__files[0])
#define stdout (&__files[1])
#define stderr (&__files[2])
#define fileno(x) (x - __files)

  FILE *fopen(const char *name, const char *mode);
  FILE *freopen(const char *name, const char *mode, FILE *fp);
  int fclose(FILE *fp);
  int fflush(FILE *fp);
  /* fdopen will always fail right now */
  FILE *fdopen(int fd, const char *mode);

  void setbuf(FILE *fp, char *buf);
  int  setvbuf(FILE *fp, char *buf, int mode, size_t size);
#if defined(_BSD_SOURCE)
  void setbuffer(FILE *fp, char *buf, size_t size);
  void setlinebuf(FILE *fp);
#endif
#if defined(_GNU_SOURCE) || defined(_POSIX_SOURCE)
  FILE *fmemopen(void *buf, size_t size, const char *mode);
#endif

  int fputc(int c, FILE *fp);
  int fputs(const char *s, FILE *fp);
  int puts(const char *s);
  int putc(int c, FILE *fp);
  int putchar(int c);

  int fgetc(FILE *fp);
  char *fgets(char *s, int size, FILE *fp);
  char *gets(char *buf);
  int ungetc(int c, FILE *fp);
  int getc(FILE *fp);
  int getchar(void);

  size_t fread(void *ptr, size_t size, size_t nmeb, FILE *fp);
  size_t fwrite(const void *ptr, size_t size, size_t nmeb, FILE *fp);

  int printf(const char *fmt, ...);
  int fprintf(FILE *fp, const char *fmt, ...);
  int sprintf(char *str, const char *format, ...);
  int snprintf(char *str, size_t size, const char *format, ...);
  int __simple_printf(const char *fmt, ...) _PRINTF_FUNC;
  int __simple_float_printf(const char *fmt, ...) _PRINTF_FUNC;
  void perror(const char *msg);

  int vprintf(const char *fmt, __va_list ap);
  int vfprintf(FILE *fp, const char *fmt, __va_list ap);
  int vsprintf(char *str, const char *format, __va_list ap);
  int vsnprintf(char *str, size_t size, const char *format, __va_list ap);

  int scanf(const char *fmt, ...);
  int fscanf(FILE *fp, const char *fmt, ...);
  int sscanf(const char *str, const char *fmt, ...);

  int vscanf(const char *fmt, __va_list ap);
  int vfscanf(FILE *fp, const char *fmt, __va_list ap);
  int vsscanf(const char *str, const char *fmt, __va_list ap);

  int remove(const char *filename);
  int rename(const char *oldname, const char *newname);

  void clearerr(FILE *fp);
  int feof(FILE *fp);
  int ferror(FILE *fp);

  int fseek(FILE *fp, long offset, int whence);
  long ftell(FILE *fp);
  void rewind(FILE *fp);

  int fgetpos(FILE *fp, fpos_t *pos);
  int fsetpos(FILE *fp, fpos_t *pos);

  /* prefix for temporary name */
#define P_tmpdir ""
  /* maximum length of temporary file returned by tmpnam */
#define L_tmpnam 16
  /* number of unique temporary names available */
#define TMP_MAX 0xffffff

  FILE *tmpfile(void);
  char *tmpnam(char *s);

#ifndef __PROPELLER_COG__
#define putc(x, stream)    fputc(x, stream)
#define putchar(x)         fputc(x, stdout)
#define getc(stream)       fgetc(stream)
#define getchar()          fgetc(stdin)
#endif

  /* internal functions */
  /* set up the FILE pointer in fp to point to a particular driver */
  FILE *__fopen_driver(FILE *fp, struct __driver *drv, const char *name, const char *mode);
  /* find a free FILE slot in the table, and mark it as in use with driver drv */
  FILE *__fopen_findslot(struct __driver *drv);

  /* set up a FILE pointer to do I/O from a string */
  FILE *__string_file(FILE *fp, char *str, const char *mode, size_t len);

  /* lock used to let multiple threads work together nicer */
  extern _atomic_t __stdio_lock;
#define __lock_stdio()   __lock(&__stdio_lock)
#define __unlock_stdio() __unlock(&__stdio_lock)

#if defined(__cplusplus)
}
#endif

#endif
