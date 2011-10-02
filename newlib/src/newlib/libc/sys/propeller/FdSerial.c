/**
 * @file FdSerial.c
 * Full Duplex Serial adapter module.
 *
 * Copyright (c) 2008, Steve Denson
 * See end of file for terms of use.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <machine/cog.h>

#include "FdSerial.h"
#include "propdev.h"

/**
 * start initializes and starts native assembly driver in a cog.
 * @param rxpin is pin number for receive input
 * @param txpin is pin number for transmit output
 * @param mode is interface mode. see header FDSERIAL_MODE_...
 * @param baudrate is frequency of bits ... 115200, 57600, etc...
 * @returns non-zero on success
 */
int FdSerial_start(FdSerial_t *data, int rxpin, int txpin, int mode, int baudrate)
{
    extern uint8_t FullDuplexSerial_firmware_array[];
    extern int FullDuplexSerial_firmware_size;
    memset(data, 0, sizeof(FdSerial_t));
    data->rx_pin  = rxpin;                  // receive pin
    data->tx_pin  = txpin;                  // transmit pin
    data->mode    = mode;                   // interface mode
    data->ticks   = _clkfreq / baudrate;    // baud
    data->buffptr = (int)&data->rxbuff[0];
    data->cogId = cognew(FullDuplexSerial_firmware_array, data) + 1;
    waitcnt(_clkfreq + _CNT);
    return data->cogId;
}

/**
 * stop stops the cog running the native assembly driver 
 */
void FdSerial_stop(FdSerial_t *data)
{
    if(data->cogId > 0) {
        cogstop(data->cogId - 1);
        data->cogId = 0;
    }
}
/**
 * rxflush empties the receive queue 
 */
void FdSerial_rxflush(FdSerial_t *data)
{
    while(FdSerial_rxcheck(data) >= 0)
        ; // clear out queue by receiving all available 
}

/**
 * Gets a byte from the receive queue if available
 * Function does not block. We move rxtail after getting char.
 * @returns receive byte 0 to 0xff or -1 if none available 
 */
int FdSerial_rxcheck(FdSerial_t *data)
{
    int rc = -1;
    if(data->rx_tail != data->rx_head) {
        rc = data->rxbuff[data->rx_tail];
        data->rx_tail = (data->rx_tail+1) & FDSERIAL_BUFF_MASK;
    }
    return rc;
}

/**
 * Wait for a byte from the receive queue. blocks until something is ready.
 * @returns received byte 
 */
int FdSerial_rx(FdSerial_t *data)
{
    int rc = FdSerial_rxcheck(data);
    while(rc < 0)
        rc = FdSerial_rxcheck(data);
    return rc;
}

/**
 * tx sends a byte on the transmit queue.
 * @param txbyte is byte to send. 
 */
int FdSerial_tx(FdSerial_t *data, int txbyte)
{
    int rc = -1;
    char* txbuff = data->txbuff;

    while(data->tx_tail != data->tx_head) // wait for queue to be empty
        ;

    //while(data->tx_tail == ((data->tx_head+1) & FDSERIAL_BUFF_MASK)) ; // wait for queue to be empty
    txbuff[data->tx_head] = txbyte;
    data->tx_head = (data->tx_head+1) & FDSERIAL_BUFF_MASK;
    if(data->mode & FDSERIAL_MODE_IGNORE_TX_ECHO)
        rc = FdSerial_rx(data); // why not rxcheck or timeout ... this blocks for char
    //wait(5000);
    return rc;
}

/*
+------------------------------------------------------------------------------------------------------------------------------+
¦                                                   TERMS OF USE: MIT License                                                  ¦                                                            
+------------------------------------------------------------------------------------------------------------------------------¦
¦Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    ¦ 
¦files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    ¦
¦modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software¦
¦is furnished to do so, subject to the following conditions:                                                                   ¦
¦                                                                                                                              ¦
¦The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.¦
¦                                                                                                                              ¦
¦THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          ¦
¦WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         ¦
¦COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   ¦
¦ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         ¦
+------------------------------------------------------------------------------------------------------------------------------+
*/
