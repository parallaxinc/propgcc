/**
 * @file FdSerial_t.c
 * Full Duplex Serial adapter module.
 *
 * Copyright (c) 2008, Steve Denson
 * See end of file for terms of use.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cog.h>
#include <propeller.h>

#include "fds.h"

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
    use_cog_driver(FullDuplexSerial);

    memset(data, 0, sizeof(FdSerial_t));
    data->rx_pin  = rxpin;                  // receive pin
    data->tx_pin  = txpin;                  // transmit pin
    data->mode    = mode;                   // interface mode
    data->ticks   = _clkfreq / baudrate;    // baud
    data->buffptr = (int)&data->rxbuff[0];
    data->cogId = load_cog_driver(FullDuplexSerial, data) + 1;

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

/*
 * Wait for a byte from the receive queue. blocks until something is ready.
 * @returns received byte 
 */
int FdSerial_rx(FdSerial_t *term)
{
  int rc = FdSerial_rxcheck(term);
  while(rc < 0)
      rc = FdSerial_rxcheck(term);
  return rc;
}

/*
 * tx sends a byte on the transmit queue.
 * @param txbyte is byte to send. 
 */
int FdSerial_tx(FdSerial_t *term, int txbyte)
{
  int rc = -1;
  char* txbuf = (char*) term->buffptr + FDSERIAL_BUFF_MASK+1;

  while(term->tx_tail == ((term->tx_head+1) & FDSERIAL_BUFF_MASK))
      ; // wait for queue to be empty
  txbuf[term->tx_head] = txbyte;
  term->tx_head = (term->tx_head+1) & FDSERIAL_BUFF_MASK;
  return rc;
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
 * drain waits for all bytes to be sent
 */
void
FdSerial_drain(FdSerial_t *data)
{
  while(data->tx_tail != data->tx_head)
    ;
  /* wait for transmission */
  waitcnt(_clkfreq + data->ticks);
}
