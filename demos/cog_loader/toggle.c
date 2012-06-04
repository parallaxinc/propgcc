/**
 * @file toggle.c
 * This program demonstrates starting COG code with C in it
 * from C. The cog makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <stdio.h>  // using temporary stdio
#include <propeller.h>
#include "toggle.h"
#include "i2c.h"
#include "cogload.h"

/*
 * This is the structure which we'll pass to the C cog.
 * It contains a small stack for working area, and the
 * mailbox which we use to communicate. See toggle.h
 * for the definition/
 */
struct par {
  unsigned stack[STACK_SIZE];
  struct toggle_mailbox m;
};

#define usefw(fw)           extern unsigned char _load_start_ ## fw ## drv[];        	\
                            extern unsigned char _load_stop_ ## fw ## drv[]

#define startcog(fw, arg)   cognewFromBootEeprom(                                       \
                                _load_start_ ## fw ## drv,                              \
                                _load_stop_ ## fw ## drv - _load_start_ ## fw ## drv,   \
                                arg)

usefw(toggle_fw_0);
usefw(toggle_fw_1);
usefw(toggle_fw_2);
usefw(toggle_fw_3);

struct par par_0;
struct par par_1;
struct par par_2;
struct par par_3;

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
    /* set up the parameters for the C cogs */
    par_0.m.wait_time = _clkfreq;  /* start by waiting for 1 second */
    par_1.m.wait_time = _clkfreq>>2;  /* start by waiting for 1/2 second */
    par_2.m.wait_time = _clkfreq>>4;  /* start by waiting for 1/4 second */
    par_3.m.wait_time = _clkfreq>>8;  /* start by waiting for 1/8 second */
    
    /* start the new cogs */
    startcog(toggle_fw_0, &par_0.m);
    startcog(toggle_fw_1, &par_1.m);
    startcog(toggle_fw_2, &par_2.m);
    startcog(toggle_fw_3, &par_3.m);
    printf("toggle cogs have started\n");

    /* every 2 seconds update the flashing frequency so the
       light blinks faster and faster */
    while(1) {
      sleep(2);
      par_0.m.wait_time >>= 1;
      if (par_0.m.wait_time < MIN_GAP)
        par_0.m.wait_time = _clkfreq;
      par_1.m.wait_time >>= 1;
      if (par_1.m.wait_time < MIN_GAP)
        par_1.m.wait_time = _clkfreq;
      par_2.m.wait_time >>= 1;
      if (par_2.m.wait_time < MIN_GAP)
        par_2.m.wait_time = _clkfreq;
      par_3.m.wait_time >>= 1;
      if (par_3.m.wait_time < MIN_GAP)
        par_3.m.wait_time = _clkfreq;
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
