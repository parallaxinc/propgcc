/**
 * @file toggle.c
 * This program demonstrates starting COG code with C in it
 * from C. The cog makes pin 15 toggle (adjust below for different
 * pins)
 *
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <stdio.h>  // using temporary stdio
#include <propeller.h>
#include "toggle.h"

/*
 * This is the structure which we'll pass to the C cog.
 * It contains a small stack for working area, and the
 * mailbox which we use to communicate. See toggle.h
 * for the definition/
 */
struct {
  unsigned stack[STACK_SIZE];
  struct toggle_mailbox m;
} par __attribute__((section(".hub")));

/*
 * function to start up a new cog running the toggle
 * code (which we've placed in the toggle_fw.cog section)
 */
void start(void *parptr)
{
    extern unsigned int _load_start_toggle_fw_cog[];
#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
    load_cog_driver_xmm(_load_start_toggle_fw_cog, 496, (uint32_t *)parptr);
#else
    cognew(_load_start_toggle_fw_cog, parptr);
#endif
}

/*
 * togglecount counts how many times the LED has been toggled
 */
volatile int togglecount = 0;

/*
 * main code
 * This is the code running in the LMM cog (cog 0).
 * It launches another cog to actually run the 
 * toggling code
 */
#define MIN_GAP 400000

void main (int argc,  char* argv[])
{
    printf("hello, world!\n");

    /* set up the parameters for the C cog */
    par.m.wait_time = _clkfreq;  /* start by waiting for 1 second */

#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
    /* warning: in XMM mode we need to leave the pins that run the external
       memory alone!
    */
    par.m.pins = 0x8000;   /* only toggle the C3 LED */
#else
    par.m.pins = 0x3fffffff;   /* toggle all pins except serial */
#endif
    /* start the new cog */
    start(&par.m);
    printf("toggle cog has started\n");

    /* every 2 seconds update the flashing frequency so the
       light blinks faster and faster */
    while(1) {
      sleep(2);
      par.m.wait_time =  par.m.wait_time >> 1;
      if (par.m.wait_time < MIN_GAP)
	par.m.wait_time = _clkfreq;
      printf("toggle count = %d\n", togglecount);
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
