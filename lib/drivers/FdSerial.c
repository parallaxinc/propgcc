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
#include <cog.h>
#include <sys/driver.h>
#include <propeller.h>

#include "FdSerial.h"

static FdSerial_t *coglist;

/**
 * start initializes and starts native assembly driver in a cog.
 * @param rxpin is pin number for receive input
 * @param txpin is pin number for transmit output
 * @param mode is interface mode. see header FDSERIAL_MODE_...
 * @param baudrate is frequency of bits ... 115200, 57600, etc...
 * @returns non-zero on success
 */
int _FdSerial_start(FdSerial_t *data, int rxpin, int txpin, int mode, int baudrate)
{
    use_cog_driver(FullDuplexSerial);

    memset(data, 0, sizeof(FdSerial_t));
    data->rx_pin  = rxpin;                  // receive pin
    data->tx_pin  = txpin;                  // transmit pin
    data->mode    = mode;                   // interface mode
    data->ticks   = _clkfreq / baudrate;    // baud
    data->buffptr = (int)&data->rxbuff[0];
    data->cogId = load_cog_driver(FullDuplexSerial, data) + 1;
    data->users = 1;

    return data->cogId;
}

/**
 * stop stops the cog running the native assembly driver 
 */
void _FdSerial_stop(FdSerial_t *data)
{
    if(data->cogId > 0) {
        cogstop(data->cogId - 1);
        data->cogId = 0;
    }
}
/**
 * rxflush empties the receive queue 
 */
void _FdSerial_rxflush(FdSerial_t *data)
{
    // clear out queue by receiving all available 
    while(_FdSerial_rxcheck(data) >= 0)
      ;
}

/**
 * Gets a byte from the receive queue if available
 * Function does not block. We move rxtail after getting char.
 * @returns receive byte 0 to 0xff or -1 if none available 
 */
int _FdSerial_rxcheck(FdSerial_t *data)
{
    int rc = -1;
    if(data->rx_tail != data->rx_head) {
        rc = data->rxbuff[data->rx_tail];
        data->rx_tail = (data->rx_tail+1) & FDSERIAL_BUFF_MASK;
    }
    return rc;
}

/**
 * Wait for a byte from the receive queue. blocks until something is ready
 * (unless _IONONBLOCK is set in the flags, in which case it returns EOF
 * immediately if no data is ready)
 * @returns received byte 
 */
int _FdSerial_getbyte(FILE *fp)
{
  FdSerial_t *data = (FdSerial_t *)fp->drvarg[0];
  int rc = _FdSerial_rxcheck(data);
  while(rc < 0 && !(fp->_flag & _IONONBLOCK)) {
    (*__yield_ptr)();
    rc = _FdSerial_rxcheck(data);
  }
  return rc;
}

/**
 * putbyte sends a byte on the transmit queue.
 * @param txbyte is byte to send. 
 */
int _FdSerial_putbyte(int txbyte, FILE *fp)
{
    FdSerial_t *data = (FdSerial_t *)fp->drvarg[0];
    volatile char* txbuff = data->txbuff;

    while(data->tx_tail == ((data->tx_head+1) & FDSERIAL_BUFF_MASK)) // wait for space in queue
        ;

    //while(data->tx_tail == ((data->tx_head+1) & FDSERIAL_BUFF_MASK)) ; // wait for queue to be empty
    txbuff[data->tx_head] = txbyte;
    data->tx_head = (data->tx_head+1) & FDSERIAL_BUFF_MASK;
    if(data->mode & FDSERIAL_MODE_IGNORE_TX_ECHO)
        _FdSerial_getbyte(fp); // why not rxcheck or timeout ... this blocks for char
    //wait(5000);
    return txbyte;
}

/**
 * drain waits for all bytes to be sent
 */
void
_FdSerial_drain(FdSerial_t *data)
{
  unsigned int waitcycles;
  while(data->tx_tail != data->tx_head)
    ;

  // wait for character to be transmitted
  // strictly speaking we only need to wait 10*data->ticks,
  // but give ourselves a bit of margin here
  waitcycles = getcnt() + (data->ticks<<4);
  waitcnt(waitcycles);
}

/*
 * standard driver interface
 */
/* globals that the loader may change; these represent the default
 * pins to use
 */
extern unsigned int _rxpin;
extern unsigned int _txpin;
extern unsigned int _baud;

static int
fdserial_fopen(FILE *fp, const char *name, const char *mode)
{
  unsigned int baud = _baud;
  unsigned int txpin = _txpin;
  unsigned int rxpin = _rxpin;
  int setBaud = 0;
  FdSerial_t *data;
  int r;

  if (name && *name) {
    baud = atoi(name);
    setBaud = 1;
    while (*name && *name != ',') name++;
    if (*name) {
      name++;
      rxpin = atoi(name);
      while (*name && *name != ',') name++;
      if (*name)
	{
	  name++;
	  txpin = atoi(name);
	}
    }
  }

  /* look for an existing cog that handles these pins */
  for (data = coglist; data; data = data->next)
    {
      if (data->tx_pin == txpin || data->rx_pin == rxpin)
	{
	  // if the baud differs, tell the cog to change it
	  if (setBaud) {
	    data->ticks = _clkfreq / baud;
	  }
	  data->users++;
	  break;
	}
    }

  if (!data)
    {
      data = hubmalloc(sizeof(FdSerial_t));
      if (!data)
	return -1;
      r = _FdSerial_start(data, rxpin, txpin, 0, baud);
      if (r <= 0)
	{
	  hubfree(data);
	  return -1;
	}

      data->next = coglist;
      coglist = data;
    }
  fp->drvarg[0] = (unsigned long)data;
  fp->_flag |= _IODEV;
  return 0;
}

static int
fdserial_fclose(FILE *fp)
{
  FdSerial_t *data = (FdSerial_t *)fp->drvarg[0];
  FdSerial_t **prev_p, *p;
  fp->drvarg[0] = 0;

  data->users--;
  if (data->users > 0)
    {
      /* still other open handles */
      return 0;
    }

  /* wait for all data to be transmitted */
  _FdSerial_drain(data);

  /* now stop */
  _FdSerial_stop(data);

  /* remove from the list */
  prev_p = &coglist;
  p = coglist;
  while (p)
    {
      if (p == data)
	{
	  *prev_p = p->next;
	  break;
	}
      prev_p = &p->next;
      p = *prev_p;
    }

  /* release memory */
  hubfree(data);
  return 0;
}

const char _FullSerialName[] = "FDS:";

_Driver _FullDuplexSerialDriver =
  {
    _FullSerialName,
    fdserial_fopen,
    fdserial_fclose,
    _term_read,
    _term_write,
    NULL,       /* seek, not needed */
    NULL,       /* remove */
    _FdSerial_getbyte,
    _FdSerial_putbyte,
  };

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
