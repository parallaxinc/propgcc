/**
 * @file toggle.c
 * This program demonstrates starting a PASM COG
 * from C. The PASM makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011, Steve Denson
 * MIT Licensed
 */

//#include <stdio.h>
#include "stdio.h"  // using temporary stdio
#include "propeller.h"

static unsigned int pins;

void start(pinptr)
{
    extern unsigned int binary_toggle_dat_start;
    cognew(&binary_toggle_dat_start, pinptr);
}

void main (int argc,  char* argv[])
{
    int n;
    int result;
    unsigned int startTime;
    unsigned int endTime;
    unsigned int executionTime;
    unsigned int rawTime;

    printf("hello, world!\n");
    pins = 0x3fffffff;
    start(&pins);
    printf("goodbyte, world!\n");
    while(1);
}
