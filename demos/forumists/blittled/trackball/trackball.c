/*
*--------------------------------------------------------------------------------------------
* Title:    Trackball Demo
* Author:    Brian L. Little
* Date:      11/13/2011
* Licensing: See end of program listing
*
* Hardware Requirements: Parallax Trackball (27908)
* Test Board:            Mini Development Board
* Compatible Boards:     Any
*
* Comments:
* This is a simple demo showing how to use Parallax's trackball with PropGCC
* The switch should be set up using any group of 5 ports starting with the basePin 
* The wiring is as follows:
*
*    Port         Pin on Switch   Description
*    ----------------------------------------
*                 Pin 1          +5V
*    basePin + 4  Pin 2           Up
*    basePin + 3  Pin 3           Center
*    basePin + 2  Pin 4           Left  
*                 Pin 5           Ground
*    basePin + 1  Pin 6           Down      
*    basePin + 5  Pin 7           LED
*    basePin      Pin 8          Right     
*--------------------------------------------------------------------------------------------
*/ 
#include "propeller.h"
#include "stdio.h"
#include <cog.h>

const int basePin = 0;            // Base pin for the 6 pin grouping 

void main (int argc,  char* argv[])
{
  int mask = 0x0000001f << basePin;    // This masks only the input pins of the grouping through
  int ledMask = 0x00000020 << basePin;  // Masks out the pin with the LED
  int offMask = 0xffffffdf << basePin;  // Mask to turn off LED
  int temp[5];                          // This array holds the value of a certain direction
  int i;                                // Loop counter
  int state = 0;            // Holds the last value from the switch contacts
  int a = 0;                // Holds the computed value from the switch contacts
  int b = 0;                        // Holds the current value from the switch contacts
  int ledDelay = 0;                     // Holds the delay count on how long the LED is on

  DIRA |= ledMask;                      // Set the LED Pin as an output

  printf("\r\n\r\nTrackball Demo\r\n");

  state = INA & mask;            // Read in the inputs and mask it for switch contacts
  
  for(;;)                // Loop continously
  {
    b = INA & mask;                    // Read in inputs
    a = b ^ state;                      // Xor with last reading to see if there is a change 
    waitcnt(CLKFREQ/20000 + CNT);       // 20 mSec delay for debouncing the switch for next reading
    
    if (ledDelay > 100)                 // Is led on deley greter than 100
    {
      OUTA |= ledMask;                  // Turn on LED
      ledDelay--;                       // Decrease LED delay count
      if (ledDelay < 0)                 // Make sure ledDelay doesn't go below 0
      {
    ledDelay = 0;
      }
    }
    else                                // if led on delay < 101
    {
      OUTA &= offMask;            // Turn off LED
    }
    if (a != 0)                // Reading will be non-zero if there is a change from
                                    // the previous reating
    {
      state = b;                // Set the last reading to be the current value
      a = a >> basePin;                 // Shift current reading so it can be read into an array
      ledDelay = 3500;                  // Set LED delay
      for(i = 0; i < 5; i++)        // Put each switch state into an array value
      {
        temp[i] = a & 0x00000001;       // Mask only one switch in the lsb of the current value
                                        // and put in the array    
        a = a >> 1;                     // Shift current value so next switch is in lsb
      }
      
      if(temp[0] == 1)                  // If temp[0] (Right) is high then display Right
        printf("Right\r\n");
      
      if(temp[1] == 1)            // If temp[1] (Down) is high then display Down
        printf("Down\r\n");

      if(temp[2] == 1)                  // If temp[2] (Left) is high then display Left
        printf("Left\r\n");

      if(temp[3] == 1)                  // If temp[3] (Center) is high then display Center          
        printf("Center\r\n");

      if(temp[4] == 1)                  // If temp[4] (Up) is high then display Up
        printf("Up\r\n");

    }
  }    
}
/*
+--------------------------------------------------------------------
¦  TERMS OF USE: MIT License
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
+------------------------------------------------------------------
*/
