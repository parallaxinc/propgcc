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

  typedef long fpos_t;

  struct _FILE {
#define _SFEOF 0x01  /* EOF seen on stream */
#define _SFERR 0x02  /* error seen on stream */
#define _SFINP 0x04  /* stream is open for input */
#define _SFOUT 0x08  /* stream is open for output */
#define _SFAPPEND 0x10 /* append data to end of file */
#define _SFUNGET 0x20 /* the ungetc character is valid */
    unsigned int flags;
    unsigned char ungetc;

    /* driver for this file */
    struct __driver *drv;

    /* auxiliary information that the driver may need */
    unsigned int drvarg[4];

    /* putc and getc functions, from the driver */
    /* we cache them here so that we can optimize away checks
       for read/write allowed; if reading is not allowed,
       for example, getbyte() will be a function that just
       returns EOF
    */
    int (*getbyte)(FILE *fp);
    int (*putbyte)(int c, FILE *fp);

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
  int ungetc(int c, FILE *fp);

#if defined(printf)
#undef printf
#endif

  int printf(const char *fmt, ...);
  int fprintf(FILE *fp, const char *fmt, ...);
  int sprintf(char *str, const char *format, ...);
  int snprintf(char *str, size_t size, const char *format, ...);
  int __simple_printf(const char *fmt, ...) _PRINTF_FUNC;
  int __simple_float_printf(const char *fmt, ...) _PRINTF_FUNC;

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

#if defined(__cplusplus)
}
#endif

#endif
