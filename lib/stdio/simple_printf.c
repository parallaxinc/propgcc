/*
 * very simple printf, adapted from one written by me
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

/*
 * very simple printf -- just understands a few format features
 * does c,s,u,d,x
 */

/*
 * define FLOAT_SUPPORT to get floating point support
 */
/*#define FLOAT_SUPPORT*/
/*
 * similarly, define LONGLONG_SUPPORT for long long support
 * (needed for FLOAT_SUPPORT)
 */
#ifdef FLOAT_SUPPORT
#define LONGLONG_SUPPORT
#endif

#ifdef LONGLONG_SUPPORT
#define ULONG unsigned long long
#define LONG long long
#else
#define ULONG unsigned long
#define LONG long
#endif

static int
PUTC(int c, int width) {
	int put = 0;

	putchar(c); put++;
	while (--width > 0) {
		putchar(' ');
		put++;
	}
	return put;
}

static int
PUTS(const char *s, int width) {
	int put = 0;

	while (*s) {
	  putchar(*s++); put++;
	  width--;
	}
	while (width-- > 0) {
	  putchar(' '); put++;
	}
	return put;
}

static int
PUTL(ULONG u, int base, int width, int fill_char)
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
	  putchar(fill_char); put++;
	}
	while (t != obuf) {
	  putchar(*--t); put++;
	}
	return put;
}

#ifdef FLOAT_SUPPORT
/* not terribly great floating point support, but it will do */
/* this is not at all standards compliant! */

#define MAX_PRECISION 18
static double
precmult[MAX_PRECISION+1] = {
    1.0, 10.0, 100.0, 1000.0, 10000.0,
    1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
    1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18
};

static int
PUTFLOAT(char c, double d, int width, int prec, int fill_char)
{
    int outbytes = 0;
    int digits;
    double power_of_ten;
    int exp;
    int sign = 0;
    ULONG integer_part, frac_part;
    const ULONG max_integer = 4000000000000000000ULL;
    if (d < 0.0) {
        sign = '-';
        d = -d;
    }
    /* figure out the integer part */
    if (d > (double)max_integer) {
        if (sign) outbytes += PUTC(sign,1);
        outbytes += PUTS("<number too big for this printf>", width-1);
        return outbytes;
    }
    integer_part = (ULONG)d;
    d -= (double)integer_part;
    if (prec < 0) prec = 0;
    if (prec > MAX_PRECISION) prec = MAX_PRECISION;
    d *= precmult[prec];
    d += 0.5;
    frac_part = (ULONG)d;
    if (frac_part >= max_integer) {
        integer_part++;
        frac_part -= max_integer;
    }
    width -= (prec+1); 
    if (sign) {
        outbytes += PUTC(sign,1); width--;
    }
    if (width < 0) width = 0;
    outbytes += PUTL(integer_part,10,width,fill_char);
    outbytes += PUTC('.',1);
    if (prec > 0)
        outbytes += PUTL(frac_part, 10, prec, '0');
    return outbytes;
}
#endif

static int
_doprnt( const char *fmt, va_list args )
{
   char c, fill_char;
   char *s_arg;
   unsigned int i_arg;
   ULONG l_arg;
   int width, long_flag;
   int outbytes = 0;
   int base;
   int prec;

   while( (c = *fmt++) != 0 ) {
     if (c != '%') {
       outbytes += PUTC(c, 1);
       continue;
     }
     c = *fmt++;
     width = 0;
     prec = -1;
     long_flag = 0;
     fill_char = ' ';
     if (c == '0') fill_char = '0';
     while (c && isdigit(c)) {
       width = 10*width + (c-'0');
       c = *fmt++;
     }
#ifdef FLOAT_SUPPORT
     if (c == '.') {
         c = *fmt++;
         prec = 0;
         while (c && isdigit(c)) {
             prec = 10*prec + (c-'0');
             c = *fmt++;
         }
     }
#endif
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
       outbytes += PUTC(c, width);
       break;
     case 'c':
       i_arg = va_arg(args, unsigned int);
       outbytes += PUTC(i_arg, width);
       break;
     case 's':
       s_arg = va_arg(args, char *);
       outbytes += PUTS(s_arg, width);
       break;
     case 'd':
     case 'x':
     case 'u':
       base = (c == 'x') ? 16 : 10;
#ifdef LONGLONG_SUPPORT
       if (long_flag > 1)
	 {
	   l_arg = va_arg(args, ULONG);
	 }
       else
	 {
	   i_arg = va_arg(args, unsigned int);
	   if (c == 'd') {
             l_arg = (ULONG)(LONG)i_arg;
	   } else {
	     l_arg = i_arg;
	   }
	 }
#else
       l_arg = va_arg(args, ULONG);
#endif
       if (c == 'd' && (((LONG)l_arg < 0))) {
           outbytes += PUTC('-', 1);
           width--;
           l_arg = (ULONG)(-((LONG)l_arg));
       }
       outbytes += PUTL(l_arg, base, width, fill_char);
       break;
#ifdef FLOAT_SUPPORT
     case 'f':
       {
         double d_arg = va_arg(args, double);
         if (prec < 0) prec = (width ? width/2 : 6);
         outbytes += PUTFLOAT(c, d_arg, width, prec, fill_char);
       }
       break;
#endif
     }
   }
   return outbytes;
}

#ifdef FLOAT_SUPPORT
int __simple_float_printf(const char *fmt, ...)
#else
int __simple_printf(const char *fmt, ...)
#endif
{
    va_list args;
    int r;
    va_start(args, fmt);
    r = _doprnt(fmt, args);
    va_end(args);
    return r;
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
