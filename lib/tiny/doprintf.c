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

int _doprintf(const char* fmt, va_list args, char* origBuf)
{
    char ch;
    char* buf = origBuf;

    while((ch = *fmt++) != 0)
    {
        if (ch != '%')
        {
            buf += _printf_putc(ch, buf);
            continue;
        }

        ch = *fmt++;

        int leftJust = 0;
        if (ch == '-')
        {
            leftJust = 1;
            ch = *fmt++;
        }

        int width = 0;
        char fillChar = ' ';
        if (ch == '0')
            fillChar = '0';
        while (ch && isdigit(ch))
        {
            width = 10 * width + (ch - '0');
            ch = *fmt++;
        }

        if (!ch)
            break;
        if (ch == '%')
        {
            buf += _printf_putc(ch, buf);
            continue;
        }

        unsigned long arg = va_arg(args, int);
        unsigned long cch;
        int base = 16;

        switch (ch)
        {
        case 'c':
            cch = (char)arg;
            arg = (unsigned long)&cch;
            /* Fall Through */
        case 's':
            if (leftJust)
                width = -width;
            buf += _printf_putn((const char*)arg, width, fillChar, buf);
            break;
        case 'd':
        case 'u':
            base = 10;
            /* Fall Through */
        case 'x':
            if (!width)
                width = 1;
            if (leftJust)
                width = -width;
            buf += _printf_putl((unsigned long)arg, base, (ch == 'd'), width, fillChar, buf);
            break;

        }
    }
    
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
