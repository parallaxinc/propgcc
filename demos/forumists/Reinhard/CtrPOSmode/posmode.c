/*
    Experiment with Low Level Register Access 
    
    reimay        11.10.2011
    
    propeller-elf-gcc   -o counter.elf  -Os counter.c 

*/

#include <stdio.h>
#include <propeller.h>

// for this experiment connect P0 with P1 !!!
#define P1 1<<1
#define P0 1<<0

// POS detector mode, accumulate FRQA to PHSA if P1 (input) = H
#define CRTMODE (1<<29) + 1  



void main ()
{
LOOP:
    getchar();                        // wait for any serial input
    
    DIRA |= P0;                        // P0 = OUTPUT
    OUTA &= ~P0;                    // P0 = LOW
    
    CTRA = CRTMODE;                    // set Counter Module
    FRQA = 1;                        // set FRQA Register := 1
    
    PHSA = 0;                        // clear PHSA Register
    
    OUTA |= P0;                        // P0 = HIGH; connected with P1 -> accumalate FRQA to PHSA
    waitcnt(CNT + CLKFREQ/1000);    // wait 1 ms
    OUTA &= ~P0;;                    // P0 = LOW; stop accumulator
    
    printf("%d\n",PHSA);            // send content of PHSA : result = 80112 ; 1ms = 80000 clocks @ 80 MHz + 112 clocks for VM
                                    // without -Os we have 752 clocks for VM, try it
    goto LOOP;
    
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
