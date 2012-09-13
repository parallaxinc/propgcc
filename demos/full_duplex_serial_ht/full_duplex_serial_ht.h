//--------------------------------------------------------------------------------------------------
//
// full_duplex_serial_ht.h
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

// RX and TX FIFO buffer sizes (Must be power of 2)
#define BUFFER_SIZE 32 
#define BUFFER_MASK (BUFFER_SIZE - 1) 

// Structure to pass data to the full duplex serial cog driver
struct fds_mailbox
{
        unsigned int stack[10];
        unsigned int txPin;
        unsigned int rxPin;
        unsigned int baudRate;
	char rxBuffer[BUFFER_SIZE]; // RX and TX FIFO buffers and pointers
	char txBuffer[BUFFER_SIZE];
	int  rxHead;
	int  rxTail;
	int  txHead;
	int  txTail;
};

// Start up a new cog running the full duplex serial cog
// code (which we've placed in the .coguser1 section)
void fdx_start(int tx_pin, int rx_pin, int baud);

// Recieve a char from the UART
int fdx_rx();

// Transmit a char through the UART
void fdx_tx(char c);

// Put a string to the UART
void fdx_puts(char* s);

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


