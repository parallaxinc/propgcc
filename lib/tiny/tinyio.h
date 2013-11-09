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

/*
 * Tiny IO features:
 *
 * - Extremely small memory footprint, achieved by:
 *    -- Limited subset of % formats in printf/scanf, no support for %*
 *    -- Behavior that's useful, but not guaranteed to be standard
 *    -- No wide character support (8-bit ASCII only)
 *    -- No support for FILE and other normal stdio conventions
 *    -- No support floating point
 *    -- Support for long long only through "put" and "get" function calls (see below)
 *    -- The entire libary, including long long support is less than 8K, significantly
 *       less if you don't use all the functionality.
 *
 * - Uses Simple Serial: doesn't require an extra cog, but there is a maximum
 *   baud rate for each clock frequency:
 *       5MHz: 9600
 *      10MHz: 19200
 *      20MHz: 38400
 *      40MHz: 57600
 *      80MHz: 115200
 *
 * - Thread-safe at the character level - this means the printf output from two
 *   threads could be intermixed, but the characters won't garble each other.
 *   Note: you must set _serialLock = 1 to enable this.
 *
 * - printf support:
 *    -- memory footprint: starts in less than 2K
 *    -- %s, %c, %d, %u, %x, and %% only
 *    -- width          (e.g. %5d %10c)
 *    -- fill character (e.g. %02d %08x)
 *    -- left justify   (e.g. %-d)
 *    -- sprintf, which shares the same code
 *
 * - __simple_printf support:
 *    -- memory footprint: starts in less than 1K
 *    -- %s, %c, %d, %u, %x, and %% only
 *    -- no width, fill, justify
 *    -- shared code with "put" function calls (see below)
 *
 * - put* atomic print routines available.  They support:
 *    -- memory footprint: less then 1K for the set
 *                         (excluding putfnum, which takes another 600 bytes)
 *    -- no width, fill, justify, except through putfnum
 *    -- shared code with __simple_printf 
 *    -- long long data type (minimal memory required for hex output,
 *                            1.6K extra memory needed to print decimals)
 *
 * - scanf support:
 *    -- memory footprint: less than 2K
 *    -- %s, %c, %d, %u, %x, and %% only
 *    -- width          (e.g. %5d %10c)
 *    -- sprintf, which shares the same code
 *
 * - There is *no* __simple_scanf support!
 *
 * - get* atomic input routines available.  They support:
 *    -- shared code with __simple_scanf
 *    -- memory footprint: less then 2K for the set.
 *    -- no width limit, except through getfnum
 *    -- long long data type
 *    -- safe_gets to avoid buffer overflow!
 *
 * Usage:
 *
 * - The printf/sprintf, scanf/sscanf, getchar, gets, putchar, puts, etc.
 *   functions in the Tiny IO library have the same signatures as the
 *   functions in the Standard IO library.  Therefore, you can compile
 *   using #include <stdio.h> or using #include "tinyio.h"; linking with
 *   tinyio.lib will replace the stdio functions with the much smaller
 *   versions!  (Note however, if you use the put* or get* functions,
 *   then you must #include "tinyio.h")
 *
 * - As noted above, the __simple_printf and and the put* share code
 *   with each other, but not with printf/sprintf.  So, if you sprintf,
 *   be sure to use printf instead of __simple_printf.  Furthermore,
 *   if you use the put* routines, use __simple_printf instead of
 *   printf.  Don't dispair if you need to intermix, the penalty is
 *   less than 1K.
 *
 * - In order to save memory, the functions behave in slightly differently
 *   from the Standard I/O library.  The most important were listed at
 *   the start of this header file.  Here are some additional differences:
 *
 *   - __simple_printf: returns 1, not the number of output characters
 *   - scanf returns the number of groups where scan was attempted,
 *     not the number of groups successfully scanned.
 *   - sscanf("-1", "%u", x) yields 0xFFFFFFF instead of not scanning
 *   - scanf gets an entire line of input (up to 80 chars) before scanning.
 *   - The get* numeric input functions each get a line of input
 *     (up to 80 chars) before scanning.
 *
 * - These routines do not use a static buffer, so as to be thread-safe.
 *   However, this means that 80-character line buffers and 11- or 21-character
 *   numeric parsing/printing buffers can appear on the stack.  For threads
 *   that use the Tiny I/O routines, be sure to allocate appropriate space
 *   for the thread stack (256 bytes should be enough, YMMV).
 *
 */

#ifndef __TINY_IO_H
#define __TINY_IO_H


#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define _PRINTF_FUNC __attribute__((format (printf, 1, 2)))
#define _FPRINTF_FUNC __attribute__((format (printf, 2, 3)))

#define EOF       (-1)

extern int isdigit(int c);

typedef int FILE;

//
// The following routines all return the number of characters written.
//

void __simple_printf(const char *fmt, ...);          // super small sprintf
#ifndef printf
int printf(const char *fmt, ...);                    // stdio
#endif
int sprintf(char *str, const char *format, ...);     // stdio - remember, the returned count does not include the null-terminator

int  putchar(int ch);                                // stdio
int  puts(const char* str);                          // stdio

void putstr(const char* str);                        // same as puts, but doesn't add CR-LF
void puthex(unsigned x);                             // print a number in hex
void putdec(int i);                                  // print a number in decimal
void putudec(unsigned x);                            // print an unsigned number in decimal
int  putfnum(unsigned long x,                        // print a number in a variable format, returns # characters written
             int base,                                  // the base
             int isSigned,                              // 1 = signed, 0 = unsigned (base 10 only - base 16 is always unsigned)
             int width,                                 // the minimum number of places to print (including the sign, if any)
             unsigned int fillChar);                    // the left fill character if the number is less than the minimum

void putlhex(unsigned long long x);                   // print a number in hex
void putldec(long long i);                            // print a number in decimal
void putludec(unsigned long long x);                  // print an unsigned number in decimal
int  putlfnum(unsigned long long x,                   // print a number in a variable format, returns # characters written
             int base,                                  // the base
             int isSigned,                              // 1 = signed, 0 = unsigned (base 10 only - base 16 is always unsigned)
             int width,                                 // the minimum number of places to print (including the sign, if any)
             unsigned int fillChar);                    // the left fill character if the number is less than the minimum

int scanf(const char *fmt, ...);
int sscanf(const char *str, const char *fmt, ...);

int getchar();
char* gets(char* buf);
char* safe_gets(char* buf, int count);

int      getdec();
unsigned getudec();
unsigned gethex();
unsigned getfnum(int base, int isSigned, int width);

long long          getldec();
unsigned long long getludec();
unsigned long long getlhex();
unsigned long long getlfnum(int base, int isSigned, int width);

    extern unsigned int _serialLock;      // Set to 1 to enable multi-threaded I/O

#if defined(__cplusplus)
}
#endif

#endif /* __TINY_IO_H */


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
