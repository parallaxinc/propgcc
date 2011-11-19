/*
 * very simple printf, adapted from one written by me
 * for the MiNT OS long ago
 * placed in the public domain
 *   - Eric Smith
 */
#include <stdarg.h>
#include <stdio.h>
#include "cog.h"

#define _WaitCnt(a,b) __builtin_propeller_waitcnt(a,b)

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
PUTC(Putfunc put, void *arg, int c) {
	int r = 0;

	r += put(arg, c);
	return r;
}

static int
PUTS(Putfunc put, void *arg, const char *s) {
	int r = 0;

	while (*s) {
		r +=  put(arg, *s++);
	}
	return r;
}

static int
PUTL(Putfunc put, void *arg, unsigned long u, int base, int width, int fill_char)
{
	int r = 0;
	static char obuf[32];
	char *t;

	t = obuf;

	do {
		*t++ = "0123456789ABCDEF"[u % base];
		u /= base;
		width--;
	} while (u > 0);

	while (width-- > 0) {
		r += put(arg, fill_char);
	}
	while (t != obuf) {
		r += put(arg, *--t);
	}
	return r;
}

static int
_doprnt( Putfunc put, void *putarg, const char *fmt, va_list args )
{
	char c, fill_char;
	char *s_arg;
	int i_arg;
	long l_arg;
	int width;
	int outbytes = 0;

	while( (c = *fmt++) != 0 ) {
		if (c != '%') {
			outbytes += PUTC(put, putarg, c);
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
			outbytes += PUTC(put, putarg, c);
			break;
		case 'c':
			i_arg = va_arg(args, int);
			outbytes += PUTC(put, putarg, i_arg);
			break;
		case 's':
			s_arg = va_arg(args, char *);
			outbytes += PUTS(put, putarg, s_arg);
			break;
		case 'd':
		case 'u':
            l_arg = va_arg(args, int);
			if (l_arg < 0 && c == 'd') {
				outbytes += PUTC(put, putarg, '-');
				width--;
				l_arg = -l_arg;
			}
			outbytes += PUTL(put, putarg, l_arg, 10, width, fill_char);
			break;
		case 'x':
            l_arg = va_arg(args, unsigned int);
			outbytes += PUTL(put, putarg, l_arg, 16, width, fill_char);
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
