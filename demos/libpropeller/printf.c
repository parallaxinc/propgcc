/*
 * very simple printf, adapted from one written by me
 * for the MiNT OS long ago
 * placed in the public domain
 *   - Eric Smith
 */

#include <stdarg.h>
#include "stdio.h"
#include "cog.h"

/*
 * GCC will optimize printf to puts if it can, so we need to
 * provide a definition for it
 */
int puts(const char *str)
{
    int c;
    int r = 0;
    while ( 0 != (c = *str++) ) {
        r += _putc(c);
    }
    r += _putc('\n');
    return r;
}

/*
 * very simple printf -- just understands a few format features
 */

static int
PUTC(int c, int width) {
	int put = 0;

	put += _putc(c);
	while (--width > 0) {
		put += _putc(' ');
	}
	return put;
}

static int
PUTS(const char *s, int width) {
	int put = 0;

	while (*s) {
		put +=  _putc(*s++);
		width--;
	}
	while (width-- > 0) {
		put += _putc(' ');
	}
	return put;
}

static int
PUTL(unsigned long u, int base, int width, int fill_char)
{
	int put = 0;
	static char obuf[32];
	char *t;

	t = obuf;

	do {
		*t++ = "0123456789ABCDEF"[u % base];
		u /= base;
		width--;
	} while (u > 0);

	while (width-- > 0) {
		put += _putc(fill_char);
	}
	while (t != obuf) {
		put += _putc(*--t);
	}
	return put;
}

static int
_doprnt( const char *fmt, va_list args )
{
   char c, fill_char;
   char *s_arg;
   int i_arg;
   long l_arg;
   int width, long_flag;
   int outbytes = 0;

   while( (c = *fmt++) != 0 ) {
     if (c != '%') {
       outbytes += PUTC(c, 1);
       continue;
     }
     c = *fmt++;
     width = 0;
     long_flag = 0;
     fill_char = ' ';
     if (c == '0') fill_char = '0';
     while (c && isdigit(c)) {
       width = 10*width + (c-'0');
       c = *fmt++;
     }

     /* for us "long int" and "int" are the same size, so
	we can ignore the 'l' flag. Someday we may wish to
	support long long (ll)
     */
     while (c == 'l' || c == 'L') {
       long_flag++;
       c = *fmt++;
     }
     if (!c) break;

     switch (c) {
     case '%':
       outbytes += PUTC(c, width);
       break;
     case 'c':
       i_arg = va_arg(args, int);
       outbytes += PUTC(i_arg, width);
       break;
     case 's':
       s_arg = va_arg(args, char *);
       outbytes += PUTS(s_arg, width);
       break;
     case 'd':
       l_arg = va_arg(args, int);
       if (l_arg < 0) {
	 outbytes += PUTC('-', 1);
	 width--;
	 l_arg = -l_arg;
       }
       outbytes += PUTL(l_arg, 10, width, fill_char);
       break;
#if 0
       /* do we really care about octal numbers? */
     case 'o':
       l_arg = va_arg(args, unsigned int);
       outbytes += PUTL(l_arg, 8, width, fill_char);
       break;
#endif
     case 'x':
       l_arg = va_arg(args, unsigned int);
       outbytes += PUTL(l_arg, 16, width, fill_char);
       break;
     case 'u':
       l_arg = va_arg(args, unsigned int);
       outbytes += PUTL(l_arg, 10, width, fill_char);
       break;

     }
   }
   return outbytes;
}

int printf(const char *fmt, ...)
{
    va_list args;
    int r;
    va_start(args, fmt);
    r = _doprnt(fmt, args);
    va_end(args);
    return r;
}
