/*
*--------------------------------------------------------------------------------------------
* Title:     5 Position Switch Demo
* Author:    Brian L. Little
* Date:      11/12/2011
* Licensing: See end of program listing
*
* Hardware Requirements: Parallax 5 Position Switch (27801)
* Test Board:            Mini Development Board
* Compatible Boards:     Any
*
* Comments:
* This is a simple demo showing how to use Parallax's 5-Position Switch with PropGCC
* The switch should be set up using any group of 5 ports starting with the basePin 
* The wiring is as follows:
*
*    Port         Pin on Switch   Description
*    --------------------------------------
*   	          Pin 1           No Contact
*    basePin      Pin 2           Right
*    basePin + 1  Pin 3	          Down
*    basePin + 2  Pin 4	          Left
*                 Pin 5	          3.25V
*    basePin + 3  Pin 6	          Center
*    basePin + 4  Pin 7           Up
*                 Pin 8           Ground         
*--------------------------------------------------------------------------------------------
*/ 
#include "propeller.h"
#include "stdio.h"
#include <cog.h>

const int basePin = 0;			// Base pin for the 5 pin grouping 

void main (int argc,  char* argv[])
{
  int mask = 0x0000001f << basePin;	// This masks only the pins of the grouping through
  int xormask = 0xffffff1f << basePin;  // Since the input goes low (0) when a switch is on
                                        // it needs to be inverted to show high (1) so this
					// mask is used to invert it
  int temp[5];                          // This array holds the value of a certain direction
  int i;                                // Loop counter
  int state = 0;			// Holds the last value from the switch contacts
  int a = 0;				// Holds the computed value from the switch contacts
  int b = 0;                    	// Holds the current value from the switch contacts        

  printf("\r\n\r\n5-Position Switch Demo\r\n");

  state = INA & mask;			// Read in the inputs and mask it for switch contacts
  
  for(;;)				// Loop continously
  {
    b = INA & mask;	                // Read in inputs
    a = b ^ state;                      // Xor with last reading to see if there is a change 
    waitcnt(CLKFREQ/2000 + CNT);        // 200 mSec delay for debouncing the switch for next reading
    
    if (a != 0)				// Reading will be non-zero if there is a change from
	                                // the previous reating
    {
      state = b;	        	// Set the last reading to be the current value
      a = (b ^ xormask) >> basePin;     // Invert the current reading so switch will read high
                                        // and shift so it can be read into an array
      for(i = 0; i < 5; i++)		// Put each switch state into an array value
      {
        temp[i] = a & 0x00000001;       // Mask only one switch in the lsb of the current value
                                        // and put in the array	
        a = a >> 1;                     // Shift current value so next switch is in lsb
      }
      
      if(temp[0] == 1)                  // If temp[0] (Right) is high then display Right
        printf("Right\r\n");
      
      if(temp[1] == 1)			// If temp[1] (Down) is high then display Down
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
