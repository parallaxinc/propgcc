/*
 * very simple vsnprintf
 * adapted by David Betz from:
 * very simple printf, adapted from one written by Eric Smith
 * for the MiNT OS long ago
 * placed in the public domain
 *   - Eric Smith
 * Propeller specific adaptations
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

/*
 * very simple printf -- just understands a few format features
 * does c,s,u,d,x
 */
 
//typedef long LONG;
//typedef unsigned long ULONG;
typedef int LONG;
typedef unsigned int ULONG;

/* string buffer */
typedef struct {
    char *p;
    size_t remaining;
} STRINGBUF;

static void
STRINGBUF_putc(STRINGBUF *buf, int ch)
{
    if (buf->remaining > 1) {
        --buf->remaining;
        *buf->p++ = ch;
    }
}

static int
PUTC(STRINGBUF *buf, int c, int width) {
	int put = 0;

	STRINGBUF_putc(buf, c); put++;
	while (--width > 0) {
		STRINGBUF_putc(buf, ' ');
		put++;
	}
	return put;
}

static int
PUTS(STRINGBUF *buf, const char *s, int width) {
	int put = 0;

	while (*s) {
	  STRINGBUF_putc(buf, *s++); put++;
	  width--;
	}
	while (width-- > 0) {
	  STRINGBUF_putc(buf, ' '); put++;
	}
	return put;
}

static int
PUTL(STRINGBUF *buf, ULONG u, int base, int width, int fill_char)
{
	int put = 0;
	char obuf[24]; /* 64 bits -> 22 digits maximum in octal */ 
	char *t;

	t = obuf;

	do {
		*t++ = "0123456789ABCDEF"[u % base];
		u /= base;
		width--;
	} while (u > 0);

	while (width-- > 0) {
	  STRINGBUF_putc(buf, fill_char); put++;
	}
	while (t != obuf) {
	  STRINGBUF_putc(buf, *--t); put++;
	}
	return put;
}

int __simple_vsnprintf(char *str, size_t size, const char *fmt, va_list args)
{
   STRINGBUF buf;
   char c, fill_char;
   char *s_arg;
   unsigned int i_arg;
   ULONG l_arg;
   int width, long_flag;
   int outbytes = 0;
   int base;
   
   buf.p = str;
   buf.remaining = size;

   while( (c = *fmt++) != 0 ) {
     if (c != '%') {
       outbytes += PUTC(&buf, c, 1);
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
	we can ignore one 'l' flag; use long long if two
    'l flags are seen */
     while (c == 'l' || c == 'L') {
       long_flag++;
       c = *fmt++;
     }
     if (!c) break;

     switch (c) {
     case '%':
       outbytes += PUTC(&buf, c, width);
       break;
     case 'c':
       i_arg = va_arg(args, unsigned int);
       outbytes += PUTC(&buf, i_arg, width);
       break;
     case 's':
       s_arg = va_arg(args, char *);
       outbytes += PUTS(&buf, s_arg, width);
       break;
     case 'd':
     case 'x':
     case 'u':
       base = (c == 'x') ? 16 : 10;
       l_arg = va_arg(args, ULONG);
       if (c == 'd' && (((LONG)l_arg < 0))) {
           outbytes += PUTC(&buf, '-', 1);
           width--;
           l_arg = (ULONG)(-((LONG)l_arg));
       }
       outbytes += PUTL(&buf, l_arg, base, width, fill_char);
       break;
     }
   }
   
   *buf.p = '\0';
   
   return outbytes;
}

int __simple_snprintf(char *str, size_t size, const char *fmt, ...)
{
    int outbytes;
    va_list ap;
    va_start(ap, fmt);
    outbytes = __simple_vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return outbytes;
}

/* +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
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
