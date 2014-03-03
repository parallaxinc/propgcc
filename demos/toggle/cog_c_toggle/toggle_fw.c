/*
 * code to toggle pins
 * this is the cog C version of the code; it runs in a separate
 * cog from the LMM kernel
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <propeller.h>
#include "toggle.h"

/*
 * our local variables (placed in cog memory) for speed
 */
static _COGMEM unsigned int waitdelay;
//static _COGMEM unsigned int pins = 0x3fffffff;  /* all pins */
static _COGMEM unsigned int pins = 0x8000;  /* just some pins -- avoid pins used by XMM drivers if you want to run in XMM mode! */
static _COGMEM unsigned int nextcnt;

extern int togglecount;

_NAKED
void main (volatile struct toggle_mailbox *m)
{
  /* get which pins to toggle from parameters */
  pins = m->pins;
  _OUTA = 0;
  _DIRA = pins;

  /* figure out how long to wait the first time */
  nextcnt = _CNT + m->wait_time;

  /* loop forever, updating the wait time from the mailbox */
  for(;;) {
    waitdelay = m->wait_time;
    _OUTA ^= pins;
    togglecount++;
    nextcnt = waitcnt2(nextcnt, waitdelay);
    //waitcnt(CNT+waitdelay);
  }
}

/*
 * The load function -- call this to start the cog.
 * We put this here so that this .cog file can be pulled in from a
 * library. If you're not using libraries it doesn't matter where it
 * goes. The reason we need it for a library (.a) is subtle; the linker
 * will only add a .cog file from a library if some symbol from the .cog
 * is actually used, so we need to provide something that the main
 * program references (the __load_start_XXX_cog symbol will not work, because
 * the linker only creates that *after* it has decided it needs the .cog!)
 *
 * the "toggle_fw_code" label serves this purpose; the caller should
 * use this instead of _load_start_toggle_fw_cog[]. This is necessary
 * because the linker does not define _load_start_XXX until *after* all
 * linking decisions are made, so that symbol cannot be used to force
 * this module to be linked; we need some symbol that is already present
 * in the .cog file.
 */

extern unsigned int _load_start_toggle_fw_cog[];
const unsigned int *toggle_fw_code = _load_start_toggle_fw_cog;

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
