//--------------------------------------------------------------------------------------------------
//
// full_duplex_serial_ht_cog.c
//
// A full duplex serial driver.
//
// Uses Heater Threads.
// Runs in a Propeller COG as native code.
//
// Copyright (c) 2011 Michael Rychlik.
// MIT Licensed (see at end of file for exact terms)
//
// History:
//
// 2011-10-08    v0.1  Initial version.
// 2011-10-16    v1.0  Removed need for "fiddle factors" in RX timing.
//                     Tested at 115200 baud.
//                     Moved Heater Threads macros out to ht.h
//--------------------------------------------------------------------------------------------------

// For Cog regs (_OUTA, _INA, _DIRA..)
#include <cog.h>

// For Heater Thread Macros
#include "ht.h"

#include "full_duplex_serial_ht.h"

#define CLKFREQ (*((int*)0))

// Pointers to mail box in HUB
static _COGMEM volatile struct fds_mailbox* pm;
static _COGMEM volatile int* pTxHead;
static _COGMEM volatile int* pTxTail;
static _COGMEM volatile char* txBuffer;
static _COGMEM volatile int* pRxHead;
static _COGMEM volatile int* pRxTail;
static _COGMEM volatile char* rxBuffer;

// Serial port config
static _COGMEM int txPin;
static _COGMEM int rxPin;
static _COGMEM int bitcycles;
static _COGMEM int oneAndHalfBitcycles;

// tx_Thread variables
static _COGMEM int txBitTime;
static _COGMEM int txShiftReg;
static _COGMEM int txBitCnt;

// rx_thread variables
static _COGMEM int rxBitTime;
static _COGMEM int rxShiftReg;
static _COGMEM int rxBitCnt;
static _COGMEM int bit;
static _COGMEM int temp;

// Threads
HT_THREAD_T htTx;
HT_THREAD_T htRx;

_NATIVE void main (volatile struct fds_mailbox *m)
{
    // Copy mail box pointer for other functions use.
    pm = m;
    pRxHead = &pm->rxHead;
    pRxTail = &pm->rxTail;
    rxBuffer = pm->rxBuffer;
    pTxHead = &pm->txHead;
    pTxTail = &pm->txTail;
    txBuffer = pm->txBuffer;

    // Set up serial port
    rxPin = pm->rxPin;
    txPin = pm->txPin;
    bitcycles = CLKFREQ / pm->baudRate;
    oneAndHalfBitcycles = bitcycles + bitcycles / 2;
    _OUTA = (unsigned)1 << txPin;
    _DIRA = (unsigned)1 << txPin;

    // Set up coroutine pointers
    htRx = &&rxThread;
    htTx = &&txThread;

    // The UART transmitter coroutine.
    HT_THREAD(txThread)
    {
        // We loop forever here.
        while(1)
        {
            // Wait while the TX FIFO is empty
            HT_WAIT_UNTIL(htTx, htRx, *pTxHead != *pTxTail);

            // Load the tx shift register with a byte from tx FIFO adding start and stop bits
            txShiftReg = (txBuffer[(*pTxTail)++ & BUFFER_MASK] | 0x100) << 1;
            txBitTime = _CNT;                                          // Start timing tx bit edges from time now. 
            HT_YIELD (htTx, htRx);                                     // Give RX time to run.
            txBitCnt = 0;
            do
            {
                HT_WAIT_UNTIL (htTx, htRx, HT_TIME_AFTER(txBitTime));  // Wait for the next bit time.
                _OUTA = (txShiftReg & 1) << txPin;                     // Output a bit. 
                txBitTime += bitcycles;                                // Get next bit time.
                HT_YIELD (htTx, htRx);                                 // Give RX time to run.
                txShiftReg >>= 1;                                      // Get next bit into position.
            } while (++txBitCnt < 10);                                 // Loop for all bits.

            // Wait for end  of stop  bit time 
            HT_WAIT_UNTIL(htTx,  htRx, HT_TIME_AFTER(txBitTime));
        }
    }

    // The UART reciever coroutine.
    HT_THREAD(rxThread)
    {
        // We loop forever here.
        while(1)
        {
            // Wait until a start bit is detected
            HT_WAIT_WHILE (htRx, htTx, ((_INA >> rxPin) & 1));

            // Set up wait until halfway into first bit.
            rxBitTime = (int)_CNT + oneAndHalfBitcycles;

            rxShiftReg = 0;
            rxBitCnt = 0;        
            do
            { 
                HT_WAIT_UNTIL (htRx, htTx, HT_TIME_AFTER(rxBitTime));  // Wait until middle of bit time.
                bit = (_INA >> rxPin) & 1;                             // Read the bit 
                rxShiftReg |= bit << rxBitCnt;                         // and place in shift register.
                rxBitTime += bitcycles;                                // Set up next RX bit time.
            } while (++rxBitCnt < 8);                                  // Loop for all bits.

            HT_YIELD (htRx, htTx);                                     // Give RX time to run.

            // Write the recieved char to the RX FIFO.
            if ((*pRxHead - *pRxTail) < BUFFER_SIZE - 1)
            {
            	HT_YIELD (htRx, htTx);                                 // Give RX time to run.
                // Not full: Put char in buffer.
                rxBuffer[(*pRxHead)++ & BUFFER_MASK] = (char)rxShiftReg;
            }
            // Wait into center of Stop bit.
            HT_WAIT_UNTIL (htRx, htTx, HT_TIME_AFTER(rxBitTime));
        }
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

