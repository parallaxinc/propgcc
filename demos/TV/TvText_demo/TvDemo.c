/*
# #########################################################
# This file demonstrates starting and running a PASM TV
# driver from C.
#   
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################
*/

#include "stdio.h"
#include "cog.h"
#include "propeller.h"
#include "TvText.h"

#define USE_STDIO

#ifdef USE_STDIO
/*
 * stdio hook for tv text
 * we could probably put this right into TvText.[ch], but
 * I didn't want to make such a drastic change just yet
 * returns 1 on success, 0 on failure
 */
int
tvText_putc(int c)
{
  outchar(c);
  return 1;
}

static int (*old_putc)(int);

/*
 * hook to print both to the TV and to serial
 */
static int
both_putc(int c)
{
  return old_putc(c) && tvText_putc(c);
} 
#endif

/*
 * main program
 */

void main (int argc, char* argv[])
{
#ifdef USE_STDIO
    /* start of printing just to serial port (the default) */
    printf("hello, serial world!\r\n");

    /* start up the tvText program */
    tvText_start(12);

    /* save old putc and set up to print to both serial and TV */
    old_putc = _putc;
    _putc = both_putc;
    printf("Hello, world!\r\n");

    /* print just to TV */
    _putc = tvText_putc;
    printf("Hello TV only!");

    /* print back to serial only */
    _putc = old_putc;
    printf("goodbyte, world!\r\n");
    while(1);
#else
    printf("hello, world!");
    tvText_start(12);
    tvText_str("Hello TV!");
    printf("\r\ngoodbyte, world!\r\n");
    while(1);
#endif
}
