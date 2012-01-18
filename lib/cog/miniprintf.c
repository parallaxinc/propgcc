/*
 * very simple printf, adapted from one written by me
 * for the MiNT OS long ago
 * placed in the public domain
 *   - Eric Smith
 */
#include <stdarg.h>
#include <stdio.h>
#include "cog.h"

/*
 * very simple printf -- just understands a few format features
 */

static int
PUTC(int c) {
	putchar(c);
	return 1;
}

static int
PUTS(const char *s) {
	int r = 0;

	while (*s) {
	  putchar(*s++);
	  r++;
	}
	return r;
}

static int
PUTL(unsigned long u, int base, int width, int fill_char)
{
	int r = 0;
	static char obuf[32];
	char *t;

	t = obuf;

	do {
		*t++ = "0123456789abcdef"[u % base];
		u /= base;
		width--;
	} while (u > 0);

	while (width-- > 0) {
		putchar(fill_char);
		r++;
	}
	while (t != obuf) {
		putchar(*--t);
		r++;
	}
	return r;
}

static int
_doprnt( const char *fmt, va_list args )
{
	char c, fill_char;
	char *s_arg;
	int i_arg;
	long l_arg;
	int width;
	int outbytes = 0;

	while( (c = *fmt++) != 0 ) {
		if (c != '%') {
			outbytes += PUTC(c);
			continue;
		}
		c = *fmt++;
		width = 0;
		fill_char = ' ';
		if (c == '0') fill_char = '0';
		while (c && isdigit(c)) {
			width = 10*width + (c-'0');
			c = *fmt++;
		}
		if (!c) break;

		switch (c) {
		case '%':
			outbytes += PUTC(c);
			break;
		case 'c':
			i_arg = va_arg(args, int);
			outbytes += PUTC(i_arg);
			break;
		case 's':
			s_arg = va_arg(args, char *);
			outbytes += PUTS(s_arg);
			break;
		case 'd':
		case 'u':
		  	l_arg = va_arg(args, int);
			if (l_arg < 0 && c == 'd') {
				outbytes += PUTC('-');
				width--;
				l_arg = -l_arg;
			}
			outbytes += PUTL(l_arg, 10, width, fill_char);
			break;
		case 'x':
		        l_arg = va_arg(args, unsigned int);
			outbytes += PUTL(l_arg, 16, width, fill_char);
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
