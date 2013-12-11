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

#include <stdarg.h>
#include "tinyio.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define PRINTF_NOT_MEMORY ((char*)0x3000000)

int _printf_putc(unsigned ch, char* buf);
int _printf_puts(const char* s, char* buf);
int _printf_putn(const char* str, int width, unsigned int fillChar, char* origBuf);
int _printf_putl(unsigned long u, int base, int isSigned, int width, unsigned int fillChar, char* buf);
int _printf_putll(unsigned long long u, int base, int isSigned, int width, unsigned int fillChar, char* buf);
void _printf_putls(unsigned long u, int base, int isSigned);
int _printf_pad(int width, int used, unsigned int fillChar, char* origBuf);

int _doprintf(const char* fmt, va_list args, char* origBuf);
int _doprintf_s(const char* fmt, va_list args);

const char* _scanf_getl(const char *str, int* dst, int base, unsigned width, int isSigned);
const char* _scanf_getll(const char *str, long long* dst, int base, unsigned width, int isSigned);

int _doscanf(const char* str, const char *fmt, va_list args);

#if defined(__cplusplus)
}
#endif


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
