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

#define STACK_SIZE 16

/* stack for cog 1 */
static int cog1_stack[STACK_SIZE];

/* per-thread library variables ("Thread Local Storage") */
static struct _TLS thread_data;

/* variables that we share between cogs */
volatile unsigned int wait_time;
volatile unsigned int pins;

/*
 * here's the toggle code that runs in another cog
 */

void
do_toggle(void *arg __attribute__((unused)) )
{
  unsigned int nextcnt;

  /* get a half second delay from parameters */
  _DIRA = pins;

  /* figure out how long to wait the first time */
  nextcnt = _CNT + wait_time;

  /* loop forever, updating the wait time from the mailbox */
  for(;;) {
    _OUTA ^= pins; /* update the pins */

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
    pins = 0x3fffffff;
    wait_time = _clkfreq;  /* start by waiting for 1 second */

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
