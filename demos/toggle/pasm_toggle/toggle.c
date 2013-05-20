/**
 * @file toggle.c
 * This program demonstrates starting a PASM COG
 * and being able to pass parameters from a C program to a PASM program
 * and from PASM back to C. 
 *
 * C to PASM Mailbox example
 *
 * WARNING: This code makes all IO pins except 30/31 toggle HIGH/LOW. Check if this is OK 
 * for the board you are using.
 *
 *
 * to use:
 * from directory containing source
 * make clean
 * make
 * propeller-load -pn -t -r toggle.elf  (where n is port #)
 *
 * Copyright (c) 2011, Steve Denson, Rick Post
 * MIT Licensed - terms of use below.
 */

#include <stdio.h>
#include <propeller.h>                    // propeller specific definitions

// the STATIC HUB mailbox for communication to PASM routine 
//
static unsigned int delay; // a pointer to this gets passed to the PASM code as the PAR register
static unsigned int pins;
static int loop_cnt;
static int pasm_done;

// C stub function to start the PASM routine
// need to be able to provide the entry point to the PASM
// and a pointer to the STATIC HUB mailbox
// the cognew function in the propeller.c library returns the COG #
//
int start(unsigned int *pinptr)
{
    // The label binary_toggle_dat_start is automatically placed
    // on the cog code from toggle.dat by objcopy (see the Makefile).
    extern unsigned int binary_toggle_dat_start[];
    return cognew(&binary_toggle_dat_start, pinptr);
}

void usleep(int t)
{
    if(t < 10)  // very small t values will cause a hang
        return; // don't bother function delay is likely enough
    waitcnt((CLKFREQ/1000000)*t+getcnt());
}
// C main function
// LMM model
void main (int argc,  char* argv[])
{
    printf("hello, world!\n");            // let the lead LMM COG say hello
    delay = CLKFREQ>>1;                    // set the delay rate in the STATIC mailbox
                                        // this is actually the duty cycle of the blink 0.5 sec on, 0.5 sec off
    pins = 0x3fFFffff;                     // set the PIN mask into the STATIC mailbox
                                        // light up all pins except 30 & 31 since we don't know board config
    loop_cnt = 20;                        // number of time through the loop (20 toggles, 10 on/off cycles)
    pasm_done = 0;                        // make sure it's zero since we'll sit and wait on it to change in a few lines
    printf ("New COG# %d started.\n",start(&delay)); // start a new COG passing a pointer to the STATIC mailbox structure
    printf ("waiting for semaphore to be set by PASM code.\n");
    while (!pasm_done)
    {
      usleep(10);                        // wait for the PASM code to clear the loop counter
    }    
    printf("goodbyte, world!\n");
    while(1);                            //let the original COG sit and spin
}

/*
    +--------------------------------------------------------------------
      TERMS OF USE: MIT License
    +--------------------------------------------------------------------
    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    +--------------------------------------------------------------------
*/
