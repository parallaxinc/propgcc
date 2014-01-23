//--------------------------------------------------------------------------------------------------
//
// full_duplex_serial_ht.c
//
// Interface to full_duplex_serial_cog.c via it's mailbox.
//
// Copyright (c) 2011 Michael Rychlik.
// MIT Licensed (see at end of file for exact terms)
//
// History:
//
// 2011-10-16    v1.0  Pulled functions out of the demo program.
//
//--------------------------------------------------------------------------------------------------
#include <propeller.h>

#include "full_duplex_serial_ht.h"

// This structure is passed to the C cog.
// See full_duplex_serial_pt.h for the definition

volatile struct fds_mailbox mbox;
// FAILS static volatile struct fds_mailbox mbox;

// Start up a new cog running the full duplex serial cog
// code (which we've placed in the .coguser1 section)
void fdx_start(int tx_pin, int rx_pin, int baud)
{
// we put our code in the fds_ht_cog section (see the objcopy in
// the Makefile)
    extern unsigned int _load_start_fds_ht_cog[];

    // Set up the fdx mail box 
    mbox.txHead = 0;
    mbox.txTail = 0;
    mbox.rxHead = 0;
    mbox.rxTail = 0;
    mbox.rxPin = rx_pin;
    mbox.txPin = tx_pin;
    mbox.baudRate = baud;

    // Start fdx driver cog
    cognew (_load_start_fds_ht_cog, (void*)&mbox);
}

// Recieve a char from the UART
int fdx_rx()
{
    while (1)
    {
        // Get char from the RX FIFO is it is not empty
        if (mbox.rxHead != mbox.rxTail)
        {
            // Yes: Remove char from RX FIFO 
            return(mbox.rxBuffer[mbox.rxTail++ & BUFFER_MASK]);
        }
     }
}

// Transmit a char through the UART
void fdx_tx(char c)
{
    // Wait if the tx buffer is full
    while((mbox.txHead - mbox.txTail) >= BUFFER_SIZE - 1) ;
    // Put char in buffer.
    mbox.txBuffer[mbox.txHead++ & BUFFER_MASK] = c;	
}

// Put a string to the UART
void fdx_puts(char* s)
{
    while(*s != 0)
    {
        fdx_tx(*s++);        
    }
}
//--------------------------------------------------------------------------------------------------
// TERMS OF USE: MIT License
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

