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

static _atomic_t _txlock;

HUBTEXT int putchar(int origval)
{
    if (origval == '\n')
        putchar('\r');
    
    int value = origval;
    int txmask = 1 << _txpin;
    int txlock = _serialLock;
    
    // Add the start and stop bits
    value = (value | 256) << 1;

    if (txlock)
        __lock(&_txlock);

    // DIRA |= txmask; OUTA |= txmask;
    __asm__ volatile("or dira, %[_mask] \n\t"
                     "or outa, %[_mask]"
                     : /* nothing */
                     : [_mask] "r" (txmask));
    
    int bitcycles = _setBitCycles();
    int waitcycles = getcnt() + bitcycles;
    
    int i;
    for (i = 0; i < 10; i++)
    {
        waitcycles = waitcnt2(waitcycles, bitcycles);

        // if (value & 1) OUTA |= txmask else OUTA &= ~txmask; value = value >> 1;
        __asm__ volatile("shr %[_value],#1 wc \n\t"
                         "muxc outa, %[_mask]"
                         : [_value] "+r" (value)
                         : [_mask] "r" (txmask));
    }

    if (txlock)
    {
        // DIRA &= ~txmask; OUTA &= ~txmask;
        __asm__ volatile("andn dira, %[_mask] \n\t"
                         "andn outa, %[_mask]"
                         : /* nothing */
                         : [_mask] "r" (txmask));
        __unlock(&_txlock);
    }

    return origval;
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
