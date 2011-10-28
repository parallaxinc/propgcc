/*
# #########################################################
# This file demonstrates starting and running a PASM TV
# driver from C.
#   
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################
*/

#include <stdio.h>
#include <cog.h>
#include <propeller.h>
#include "TvText.h"

#define USE_STDIO

#ifdef USE_STDIO
extern _Driver TvDriver;
extern _Driver _SimpleSerialDriver;

/* This is a list of all drivers we can use in the
 * program. The default _InitIO function opens stdin,
 * stdout, and stderr based on the first driver in
 * the list (the serial driver, for us)
 */
_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  &TvDriver,
  NULL
};

#endif

/*
 * main program
 */

void main (int argc, char* argv[])
{
#ifdef USE_STDIO
    FILE *tvf;

    /* start of printing just to serial port (the default) */
    printf("hello, serial world!\r\n");

    /* start up the tvText program on pin 12 */
    tvf = fopen("TV:12", "w");

    /* print to both serial and TV */
    printf("Hello, world!\r\n");
    fprintf(tvf, "Hello, world\r\n");

    /* print just to TV */
    fprintf(tvf, "Hello TV only!\r\n");
    fprintf(tvf, "TV line 2!\r\n");

    /* print back to serial only */
    printf("goodbye, world!\r\n");
    while(1);
#else
    printf("hello, world!");
    tvText_start(12);
    tvText_str("Hello TV!");
    printf("\r\ngoodbye, world!\r\n");
    while(1);
#endif
}
