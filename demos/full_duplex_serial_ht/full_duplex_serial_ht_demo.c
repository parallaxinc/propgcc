//--------------------------------------------------------------------------------------------------
//
// full_duplex_serial_ht_demo.c
//
// This program demonstrates the use of full_duplex_serial_ht
//
// Copyright (c) 2011 Michael Rychlik
// MIT Licensed (see at end of file for exact terms)
//
//--------------------------------------------------------------------------------------------------
#include "stdio.h"
#include "propeller.h"
#include "full_duplex_serial_ht.h"


char jackSays[] = "All work and no play makes Jack a dull bot.\n";
char temp;
int i;

// Main code
// This is the code running in the LMM cog (cog 0).
// It launches another cog to actually run the full duplex serial code
int main (int argc,  char* argv[])
{
    // Give up the pins for the fds driver
    _DIRA = 0;
    _OUTA = 0;

    // Start the new cog
    //fdx_start(30, 31, 38400);
    //fdx_start(30, 31, 57600);
    fdx_start(30, 31, 115200);

    fdx_puts(jackSays);    

    // Echo recieved chars
    while(1)
    {
//#define ECHO_BIN
#ifdef ECHO_BIN
        temp = fdx_rx();
        for (i = 0; i < 8; i++)
        {
            if (temp & 0x80)
            {
                fdx_tx('1'); 
            }
            else
            {
                fdx_tx('0');
            }
            temp <<= 1;
        }
        fdx_tx('\n');
#else
        fdx_tx(fdx_rx());
        //fdx_puts(jackSays);    
        //fdx_tx(' ');
#endif   
    }
}
//--------------------------------------------------------------------------------------------------
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files
// (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//--------------------------------------------------------------------------------------------------
