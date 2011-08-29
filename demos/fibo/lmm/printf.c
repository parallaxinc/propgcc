/*
 * very simple printf, adapted from one written by me
 * for the MiNT OS long ago
 * placed in the public domain
 *   - Eric Smith
 */

#include <stdarg.h>
#include "stdio.h"
#include "cog.h"

#define _WaitCnt(a,b) __builtin_waitcnt(a,b)

typedef struct SerialS {
    int rxpin;
    int txpin;
    int bitcycles;
} SerialT;

static SerialT SerialPort;

int _tx(void *txarg, int value)
{
    SerialT *sp = (SerialT *)txarg;
    int i;
    int txpin = sp->txpin;
    int bitcycles = sp->bitcycles;
    int waitcycles = _CNT + bitcycles;

    value = (value | 256) << 1;
    for (i = 0; i < 10; i++)
    {
        waitcycles = _WaitCnt(waitcycles, bitcycles);
        _OUTA = (value & 1) << txpin;
        value >>= 1;
    }
}

int puts(const char *str)
{
    int c;
    int r = 0;
    while ( 0 != (c = *str++) ) {
        r += _tx(&SerialPort, c);
    }
    return r;
}

void _start_serial(int rxpin, int txpin, int baudrate)
{
    SerialPort.rxpin = rxpin;
    SerialPort.txpin = txpin;
    SerialPort.bitcycles = *((int *)0) / baudrate;
    _OUTA = 1 << txpin;
    _DIRA = 1 << txpin;
}

/*
 * very simple printf -- just understands a few format features
 */

typedef int (*Putfunc)(void *, int);

static int
PUTC(Putfunc putc, void *arg, int c, int width) {
	int put = 0;

	put += putc(arg, c);
	while (--width > 0) {
		put += putc(arg, ' ');
	}
	return put;
}

static int
PUTS(Putfunc putc, void *arg, const char *s, int width) {
	int put = 0;

	if (s == 0) s = "(null)";

	while (*s) {
		put +=  putc(arg, *s++);
		width--;
	}
	while (width-- > 0) {
		put += putc(arg, ' ');
	}
	return put;
}

static int
PUTL(Putfunc putc, void *arg, unsigned long u, int base, int width, int fill_char)
{
	int put = 0;
	static char obuf[32];
	char *t;

	t = obuf;

	do {
		*t++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[u % base];
		u /= base;
		width--;
	} while (u > 0);

	while (width-- > 0) {
		put += putc(arg, fill_char);
	}
	while (t != obuf) {
		put += putc(arg, *--t);
	}
	return put;
}

static int
_doprnt( Putfunc putc, void *putarg, const char *fmt, va_list args )
{
	char c, fill_char;
	char *s_arg;
	int i_arg;
	long l_arg;
	int width, long_flag;
    int outbytes = 0;

	while( (c = *fmt++) != 0 ) {
		if (c != '%') {
			outbytes += PUTC(putc, putarg, c, 1);
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
		if (c == 'l' || c == 'L') {
			long_flag = 1;
			c = *fmt++;
		}
		if (!c) break;

		switch (c) {
		case '%':
			outbytes += PUTC(putc, putarg, c, width);
			break;
		case 'c':
			i_arg = va_arg(args, int);
			outbytes += PUTC(putc, putarg, i_arg, width);
			break;
		case 's':
			s_arg = va_arg(args, char *);
			outbytes += PUTS(putc, putarg, s_arg, width);
			break;
		case 'd':
			if (long_flag) {
				l_arg = va_arg(args, long);
			} else {
				l_arg = va_arg(args, int);
			}
			if (l_arg < 0) {
				outbytes += PUTC(putc, putarg, '-', 1);
				width--;
				l_arg = -l_arg;
			}
			outbytes += PUTL(putc, putarg, l_arg, 10, width, fill_char);
			break;
		case 'o':
			if (long_flag) {
				l_arg = va_arg(args, long);
			} else {
				l_arg = va_arg(args, unsigned int);
			}
			outbytes += PUTL(putc, putarg, l_arg, 8, width, fill_char);
			break;
		case 'x':
			if (long_flag) {
				l_arg = va_arg(args, long);
			} else {
				l_arg = va_arg(args, unsigned int);
			}
			outbytes += PUTL(putc, putarg, l_arg, 16, width, fill_char);
			break;
		case 'u':
			if (long_flag) {
				l_arg = va_arg(args, long);
			} else {
				l_arg = va_arg(args, unsigned int);
			}
			outbytes += PUTL(putc, putarg, l_arg, 10, width, fill_char);
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
    r = _doprnt(_tx, &SerialPort, fmt, args);
    va_end(args);
    return r;
}
