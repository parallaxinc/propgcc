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
#define	_IORW		0x0080		/* file is open for update (r+w) */
#define	_IOFBF		0x0100		/* i/o is fully buffered */
#define	_IOLBF		0x0200		/* i/o is line buffered */
#define	_IONBF		0x0400		/* i/o is not buffered */
#define	_IOMYBUF	0x0800		/* standard buffer */
#define	_IOEOF		0x1000		/* EOF has been reached */
#define	_IOERR		0x2000		/* an error has occured */
#define _IOAPPEND       0x4000

    long		_cnt;		/* # of bytes in buffer */
    unsigned char	*_ptr;		/* current buffer pointer */
    unsigned char	*_base;		/* base of file buffer */
    unsigned int	_flag;		/* file status flags */
    long		_bsiz;		/* buffer size */

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

  FILE *fopen(const char *name, const char *mode);
  FILE *freopen(const char *name, const char *mode, FILE *fp);
  int fclose(FILE *fp);
  int fflush(FILE *fp);

  int fputc(int c, FILE *fp);
  int fputs(const char *s, FILE *fp);
  int puts(const char *s);

  int fgetc(FILE *fp);
  char *fgets(char *s, int size, FILE *fp);
  char *gets(char *buf);
  int ungetc(int c, FILE *fp);

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

#define putc(x, stream)    fputc(x, stream)
#define putchar(x)         fputc(x, stdout)
#define getc(x, stream)    fgetc(x, stream)
#define getchar()          fgetc(x, stdout)

  /* internal functions */
  /* set up the FILE pointer in fp to point to a particular driver */
  FILE *__fopen_driver(FILE *fp, struct __driver *drv, const char *name, const char *mode);
  /* set up a FILE pointer to do I/O from a string */
  FILE *__string_file(FILE *fp, char *str, const char *mode, size_t len);

#if defined(__cplusplus)
}
#endif

#endif
