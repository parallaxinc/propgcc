//
// file = QS_padscan.c
// propgcc conversion of JonnyMac's SPIN version of Quickstart Button Demo
// simple propgcc tutorial demo
//
// copyright 2011 by Rick Post
// terms of use below
// 
#include <propeller.h>

int scan_pads(long int delay)
{
      OUTA |= 0x000000ff;                        // "charge" the pads - force them HIGH
      DIRA |= 0x000000ff;                        // set them as output
      DIRA &= 0xffffff00;                        // release touch pads back to input
    waitcnt(delay+CNT);                     // delay for a "touch"
                                            // a touched pad goes to LOW
                                            // so if you invert the input bits you can mask them out
                                            // and return the "touched" pads as HIGHS
    return ((~INA) & 0x000000ff);             // return the touched pads
}


int main(void)
{
    int pads = 0;
    DIRA |= 0x00ff0000;                        // set LEDs for output and pads for input
    DIRA &= 0xffffff00;
    while (1)
    {
        pads = scan_pads(CLKFREQ / 100);    // scan the pads passing delay to wait for user touch
        pads = pads << 16;                    // shift the pad pins (0..7) to the LED pins (16..23)
        OUTA &= 0xff00ffff;                    // clear out the scanned pads from last time
        OUTA |= pads;                        // set the most recent scanned pads
    }
    return(0);
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
