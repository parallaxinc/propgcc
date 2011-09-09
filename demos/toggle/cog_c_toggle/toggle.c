/**
 * @file toggle.c
 * This program demonstrates starting COG code with C in it
 * from C. The cog makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011, Steve Denson and Eric R. Smith
 * MIT Licensed
 */

#include "stdio.h"  // using temporary stdio
#include "propeller.h"
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
} par;

/*
 * function to start up a new cog running the toggle
 * code (which we've placed in the .coguser1 section)
 */
void start(void *parptr)
{
    extern unsigned int _load_start_coguser1[];
    cognew(_load_start_coguser1, parptr);
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
    par.m.wait_time = _clkfreq;  /* start by waiting for 1 second */
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
    }
}
