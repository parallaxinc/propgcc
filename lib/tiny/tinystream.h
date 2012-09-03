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

/*
 * Tiny IO features:
 *
 * - Extremely small memory footprint.  On the Prop, a "Hello World" program
 *   using stdlibc++ takes 500K.  It takes less than 2K by using this header
 *   and the Tiny I/O library.
 *
 * - However, this I/O class is not link-compatible.  In other words, you must
 *   #include <tinystream.h> instead of #include <iostream>
 *
 * - operator new / operator delete without the exception overhead.  These
 *   "skinny" allocators do have the appropriate signatures and therefore
 *   are link-compatible without including any special header.
 *
 */

namespace std
{
    enum fmtflags
    {
        dec    = 1 << 0,
        hex    = 1 << 1
    };

    class stream
    {
    private:
        fmtflags flags;
        int base;

        inline void set_base (int b)
        {
            base = b;
        }

    public:
        inline stream()
        {
            flags = dec;
            base = 10;
        }

        inline stream& operator <<(unsigned long i)
        {
            _printf_putl(i, base, 0, 0, ' ', PRINTF_NOT_MEMORY);
            return *this;
        }

        inline stream& operator <<(unsigned u)
        {
            _printf_putl(u, base, 0, 0, ' ', PRINTF_NOT_MEMORY);
            return *this;
        }

        inline stream& operator <<(unsigned char u)
        {
            _printf_putl(u, base, 0, 0, ' ', PRINTF_NOT_MEMORY);
            return *this;
        }

        inline stream& operator <<(unsigned long long u)
        {
            _printf_putll(u, base, 0, 0, ' ', PRINTF_NOT_MEMORY);
            return *this;
        }

        inline stream& operator <<(long long i)
        {
            _printf_putll(i, base, 1, 0, ' ', PRINTF_NOT_MEMORY);
            return *this;
        }

        inline stream& operator <<(char c)
        {
            putchar(c);
            return *this;
        }

        inline stream& operator <<(const char* s)
        {
            putstr(s);
            return *this;
        }

        inline stream& operator <<(char* s)
        {
            putstr((const char*)s);
            return *this;
        }

        template <typename T> inline stream& operator <<(T i)
        {
            _printf_putl(i, base, 1, 0, ' ', PRINTF_NOT_MEMORY);
            return *this;
        }

        inline stream& operator <<(fmtflags f)
        {
            switch (f)
            {
            case dec:
                set_base (10);
                break;
            case hex:
                set_base (16);
                break;
            }
            return *this;
        }

        inline stream& operator >>(unsigned long& i)
        {
            i = getfnum(base, 0, 80);
            return *this;
        }

        inline stream& operator >>(unsigned& u)
        {
            u = getlfnum(base, 0, 80);
            return *this;
        }

        inline stream& operator >>(unsigned char& u)
        {
            u = getlfnum(base, 0, 80);
            return *this;
        }

        inline stream& operator >>(unsigned long long& u)
        {
            u = getlfnum(base, 0, 80);
            return *this;
        }

        inline stream& operator >>(long long& i)
        {
            i = getlfnum(base, 1, 80);
            return *this;
        }

        inline stream& operator >>(char& c)
        {
            c = getchar();
            return *this;
        }

        inline stream& operator >>(char* s)
        {
            safe_gets(s, 80);
            return *this;
        }

        template <typename T> inline stream& operator >>(T& i)
        {
            i = getfnum(base, 1, 80);
            return *this;
        }

        inline stream& operator >>(fmtflags f)
        {
            switch (f)
            {
            case dec:
                set_base (10);
                break;
            case hex:
                set_base (16);
                break;
            }
            return *this;
        }
    };

    const char endl = '\n';

    extern stream cout;
    extern stream cin;
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
