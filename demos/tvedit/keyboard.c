/**
 * @file keyboard.c - provides keyboard access via PASM
 *
 * Copyright (c) 2008, Steve Denson
 *
 * ----------------------------------------------------------------------------
 *                        TERMS OF USE: MIT License
 * ----------------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * ----------------------------------------------------------------------------
 */
 
#include <propeller.h>
#include "keyboard.h"

/*
 * All function API are described in keyboard.h
 */

#ifndef PASMLEN
#define PASMLEN 496
#endif

static int gcog; // cog for start stop
HUBDATA Keybd_st gkb;
 
int keybd_start(int dpin, int cpin)
{
    return keybd_startx(dpin, cpin, 0x04, 0x28);
}

int keybd_startx(int dpin, int cpin, int locks, int autorep)
{
    extern uint32_t binary_Keyboard_dat_start[];

    gkb.keys[0] = dpin;
    gkb.keys[1] = cpin;
    gkb.keys[2] = locks;
    gkb.keys[3] = autorep;

#if defined(__PROPELLER_XMM__)
    // use the pasm array defined in TvText.c
    {
        extern uint32_t pasm[];
        //copy_from_xmm((uint32_t*)pasm,(uint32_t*)binary_Keyboard_dat_start,PASMLEN);
        memcpy((char*)pasm,(char*)binary_Keyboard_dat_start,PASMLEN<<2);
        gcog = cognew((uint32_t)pasm, (uint32_t)&gkb) + 1;
    }
#else
    gcog = cognew((uint32_t)binary_Keyboard_dat_start, (uint32_t)&gkb) + 1;
#endif
    waitcnt((CLKFREQ>>4)+CNT); // wait for driver to start and configure keyboard
    return gcog;
}

void keybd_stop(void)
{
    int cog = gcog-1;
    if(gcog > 0) {
        cogstop(cog);
        gcog = 0;
    }
    memset((void*)&gkb, 0, sizeof(gkb)>>2);
}

int keybd_present(void)
{
    return gkb.present;
}

int keybd_key(void)
{
    int rc = 0;
    if (gkb.tail != gkb.head) {
        rc = *(short*)((short*)&gkb.keys[0]+gkb.tail);
        gkb.tail = (++gkb.tail & 0xf); // move queue pointer
    }
    return rc;
}

int keybd_getkey(void)
{
    int rc = 0;
    while(!(rc = keybd_key()))
        ; // block
    return rc;
}

int keybd_newkey(void)
{
    gkb.tail = gkb.head; // block forever until key hit
    return keybd_getkey();
}

int keybd_gotkey(void)
{
    return gkb.tail != gkb.head;
}

void keybd_clearkeys(void)
{
    gkb.tail = gkb.head; // reset queue
}

int keybd_keystate(int key)
{
    int rc = -(gkb.states[key >> 5] >> key & 1);
    return rc;
}
