/*
 * Super-simple text I/O for PropGCC, stripped of all stdio overhead.
 * Copyright (c) 2012, Ted Stefanik. Concept inspired by:
 *
 *     very simple printf, adapted from one written by me [Eric Smith]
 *     for the MiNT OS long ago
 *     placed in the public domain
 *       - Eric Smith
 *     Propeller specific adaptations
 *     Copyright (c) 2011 Parallax, Inc.
 *     Written by Eric R. Smith, Total Spectrum Software Inc.
 *
 * MIT licensed (see terms at end of file)
 */

#include "siodev.h"

static char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

int _printf_putll(unsigned long long u, int base, int isSigned, int width, unsigned int fillChar, char* origBuf)
{
	char tmpBuf[22];  // Base must be 10 or 16, so the number could be 20 digits
	tmpBuf[21] = 0;

    register char* start = &tmpBuf[20];
	register char* tmp = start;

    if (isSigned && base == 10 && (long long)u < 0)
        u = -(long long)u;
    else
        isSigned = 0;

	do
    {
        //
        // Note: this is a time-intensive way to get the remainder.
        // However, it saves 1.6K, because it avoids linking in the
        // the long long remainder subroutine.
        //
        long long origU = u;
        u = u / base;
        register int rem = origU - (u * base);
		*tmp-- = digits[rem];
	} while (u > 0);

    if (isSigned)
		*tmp-- = '-';

    int used = start - tmp;
    char* buf = origBuf;
    int tmpStart = 1;

    if (isSigned && fillChar != ' ')
    {
        buf += _printf_putc('-', buf);
        tmpStart++;
    }
	buf += _printf_pad(width, used, fillChar, buf);
    buf += _printf_puts(&tmp[tmpStart], buf);
	buf += _printf_pad(-width, used, ' ', buf);
	return buf - origBuf;
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
