/*
 * code to toggle pins
 * this is the cog C version of the code; it runs in a separate
 * cog from the LMM kernel
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <propeller.h>
#include "toggle.h"

#define MIN_GAP 400000

#define CLKFREQ_P	((uint32_t *)0)
/*
 * our local variables (placed in cog memory) for speed
 */
static _COGMEM unsigned int tokendelay;
static _COGMEM unsigned int pinmask = PIN;  /* pin number */
static _COGMEM unsigned int nextcnt;

_NATIVE
void main (volatile struct toggle_mailbox *m)
{
  /* add the base pin number and make a mask */
  pinmask = (1 << m->basepin + pinmask);
  
  /* get a half second delay from parameters */
  _DIRA = pinmask;

  /* make the token delay 1 second */
  tokendelay = *CLKFREQ_P;
  
  /* figure out how long to wait the first time */
  nextcnt = _CNT + m->wait_time;

  /* loop forever, updating the wait time from the mailbox */
  for (;;) {
    if (m->token) {
        m->wait_time >>= 1;
        if (m->wait_time < MIN_GAP)
            m->wait_time = *CLKFREQ_P;
        nextcnt = waitcnt2(_CNT + tokendelay, m->wait_time);
        m->next->token = 1;
        m->token = 0;
    }
    else {
        _OUTA ^= pinmask;
        nextcnt = waitcnt2(nextcnt, m->wait_time);
    }
  }
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
