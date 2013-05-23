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

#include <propeller.h>
#include <sys/thread.h>
#include "serialparam.h"
#include "tinyio.h"

static _atomic_t _rxlock;

HUBTEXT int getchar()
{
    int rxlock = _serialLock;
    if (rxlock)
        __lock(&_rxlock);
    
    int value = 0;
    unsigned int rxmask = 1 << _rxpin;

    /* set input */
    // dira &= ~rxmask;
    __asm__ volatile("andn dira, %[_mask]"
                     : /* nothing */
                     : [_mask] "r" (rxmask));

    unsigned int bitcycles = _setBitCycles();

    /* wait for a start bit */
    waitpeq(0, rxmask);

    /* sync for one half bit */
    unsigned int waitcycles = getcnt() + (bitcycles>>1) + bitcycles;

    int i;
    for (i = 0; i < 8; i++)
    {
        waitcycles = waitcnt2(waitcycles, bitcycles);

        // value = ( (0 != (_INA & rxmask)) << 7) | (value >> 1);
        __asm__ volatile("shr %[_value],# 1\n\t"
                         "test %[_mask],ina wz \n\t"
                         "muxnz %[_value], #1<<7"
                         : [_value] "+r" (value)
                         : [_mask] "r" (rxmask));
    }
    
    /* wait for the line to go high (as it will when the stop bit arrives) */
    __builtin_propeller_waitpeq(rxmask, rxmask);
    
    if (rxlock)
        __unlock(&_rxlock);

    return value;
}

/*
+--------------------------------------------------------------------
¦  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
