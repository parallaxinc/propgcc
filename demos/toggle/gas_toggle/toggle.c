/**
 * @file toggle.c
 * This program demonstrates starting another COG running
 * GAS code.
 * The cog makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011, Parallax, Inc.
 * MIT Licensed
 */

#include <stdio.h>
#include <propeller.h>
#include <stdlib.h>
#include <string.h>

/*
 * function to start up a new cog running the toggle
 * code (which we've placed in the .cogtoggle section)
 */
void start_cog(void)
{
    extern unsigned int _load_start_cogtoggle[];

    /* now start the kernel */
#if defined(__PROPELLER_XMMC__) || defined(__PROPELLER_XMM__)
    unsigned int *buffer;

    // allocate a buffer in hub memory for the cog to start from
    buffer = __builtin_alloca(2048);
    memcpy(buffer, _load_start_cogtoggle, 2048);
    cognew(buffer, 0);
#else
    cognew(_load_start_cogtoggle, 0);
#endif
}

/* variables that we share between cogs */
HUBDATA volatile unsigned int wait_time;
HUBDATA volatile unsigned int pins;


/*
 * main code
 * This is the code running in the LMM cog (cog 0).
 * It launches another cog to actually run the 
 * toggling code
 */
#define MIN_GAP 400000

void main (int argc,  char* argv[])
{
    int n;
    int result;
    unsigned int startTime;
    unsigned int endTime;
    unsigned int executionTime;
    unsigned int rawTime;

    printf("hello, world!\n");

    /* set up the parameters for the other cog */
    /* note that we have to avoid any pins being used by the XMM
       interpreter, if we are in XMM mode!
    */
#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
    pins = (1<<15); /* just the LED on the C3 board */
#else
    pins = 0x3fffffff;
#endif
    wait_time = _clkfreq;  /* start by waiting for 1 second */

    /* start the new cog */
    start_cog();
    printf("toggle cog has started\n");

    /* every 2 seconds update the flashing frequency so the
       light blinks faster and faster */
    while(1) {
      sleep(2);
      wait_time =  wait_time >> 1;
      if (wait_time < MIN_GAP)
	wait_time = _clkfreq;
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
