/**
 * @file toggle.c
 * This program demonstrates starting another COG running
 * LMM code.
 * The cog makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <stdio.h>
#include <propeller.h>
#include <sys/thread.h>

#ifdef __PROPELLER2__
#define DIR DIRB
#define OUT PINB
#else
#define DIR DIRA
#define OUT OUTA
#endif

#if defined(__PROPELLER_XMM__)
#define STACK_SIZE (1024+128+32) /* need room for XMM cache */
#else
#define STACK_SIZE 32
#endif

/* stack for cog 1 */
/* this must be in HUB memory */
HUBDATA static int cog1_stack[STACK_SIZE];

/* variables that we share between cogs */
/* these must go in HUB memory so as to avoid cache coherency issues */
HUBDATA volatile unsigned int wait_time;
HUBDATA volatile unsigned int pins;

/* per-thread library variables ("Thread Local Storage") */
static _thread_state_t thread_data;

/*
 * here's the toggle code that runs in another cog
 */

//__attribute__((section(".hubtext")))
void
do_toggle(void *arg __attribute__((unused)) )
{
  unsigned int nextcnt;
  unsigned tmppins = pins;

  /* figure out how long to wait the first time */
  nextcnt = getcnt() + wait_time;

  /* loop forever, updating the wait time from the mailbox */
  for(;;) {
    tmppins = pins;
    DIR = tmppins;
    OUT ^= tmppins; /* update the pins */

    /* sleep until _CNT == nextcnt, and return the new _CNT + wait_time */
    nextcnt = __builtin_propeller_waitcnt(nextcnt, wait_time);
  }
}

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

    /* set up the parameters for the C cog */
#if defined(__PROPELLER_USE_XMM__)
    pins = 0x00008000;  /* C3 led only */
#else
    pins = 0x3fffffff;
#endif
#if defined(__PROPELLER2__)
    wait_time = 30000000;
#else
    wait_time = _clkfreq;  /* start by waiting for 1 second */
#endif

    /* start the new cog */
    n = _start_cog_thread(cog1_stack + STACK_SIZE, do_toggle, NULL, &thread_data);
    printf("toggle cog %d has started\n", n);

    /* every 2 seconds update the flashing frequency so the
       light blinks faster and faster */
    while(1) {
      sleep(2);
      wait_time =  wait_time >> 1;
      if (wait_time < MIN_GAP)
	wait_time = _clkfreq;
      printf("time = %d cycles\n", wait_time);
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
