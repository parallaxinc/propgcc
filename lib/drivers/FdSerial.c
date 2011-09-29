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

#include "FdSerial.h"
#include "propdev.h"

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
    extern void *_FullDuplexCogCode;

    memset(data, 0, sizeof(FdSerial_t));
    data->rx_pin  = rxpin;                  // receive pin
    data->tx_pin  = txpin;                  // transmit pin
    data->mode    = mode;                   // interface mode
    data->ticks   = _clkfreq / baudrate;    // baud
    data->buffptr = (int)&data->rxbuff[0];
    data->cogId = cognew(_FullDuplexCogCode, data) + 1;
    data->users = 1;

    //waitcnt(_clkfreq + _CNT);
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
    while(_FdSerial_rxcheck(data) >= 0)
        ; // clear out queue by receiving all available 
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
 * Wait for a byte from the receive queue. blocks until something is ready.
 * @returns received byte 
 */
int _FdSerial_rx(FdSerial_t *data)
{
    int rc = _FdSerial_rxcheck(data);
    while(rc < 0)
        rc = _FdSerial_rxcheck(data);
    return rc;
}

/**
 * tx sends a byte on the transmit queue.
 * @param txbyte is byte to send. 
 */
int _FdSerial_tx(FdSerial_t *data, int txbyte)
{
    int rc = -1;
    char* txbuff = data->txbuff;

    while(data->tx_tail == ((data->tx_head+1) & FDSERIAL_BUFF_MASK)) // wait for space in queue
        ;

    //while(data->tx_tail == ((data->tx_head+1) & FDSERIAL_BUFF_MASK)) ; // wait for queue to be empty
    txbuff[data->tx_head] = txbyte;
    data->tx_head = (data->tx_head+1) & FDSERIAL_BUFF_MASK;
    if(data->mode & FDSERIAL_MODE_IGNORE_TX_ECHO)
        rc = _FdSerial_rx(data); // why not rxcheck or timeout ... this blocks for char
    //wait(5000);
    return rc;
}

/**
 * drain waits for all bytes to be sent
 */
void
_FdSerial_drain(FdSerial_t *data)
{
  while(data->tx_tail != data->tx_head)
    ;
  /* wait for transmission */
  waitcnt(_clkfreq + data->ticks);
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
  FdSerial_t *data;
  int r;

  if (name && *name) {
    baud = atoi(name);
    while (*name && *name != ',') name++;
    if (*name) {
      name++;
      rxpin = atoi(name);
      while (*name && *name != ',') name++;
      if (*name)
	txpin = atoi(name);
    }
  }

  /* look for an existing cog that handles these pins */
  for (data = coglist; data; data = data->next)
    {
      if (data->tx_pin == txpin || data->rx_pin == rxpin)
	break;
    }

  if (!data)
    {
      data = malloc(sizeof(FdSerial_t));
      if (!data)
	return -1;
      r = _FdSerial_start(data, rxpin, txpin, 0, baud);
      if (r <= 0)
	{
	  free(data);
	  return -1;
	}

      data->next = coglist;
      coglist = data;
    }
  fp->drvarg[0] = (unsigned long)data;
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
  free(data);
  return 0;
}

static int
fdserial_read(FILE *fp, unsigned char *buf, int size)
{
  int count = 0;
  int c;
  FdSerial_t *data = (FdSerial_t *)fp->drvarg[0];
  while (count < size) {
    buf[count] = c = _FdSerial_rx(data);
    ++count;
    if (c == '\n') break;
  }
  return count;
}

static int
fdserial_write(FILE *fp, unsigned char *buf, int size)
{
  int count = 0;
  FdSerial_t *data = (FdSerial_t *)fp->drvarg[0];
  while (count < size) {
    _FdSerial_tx(data, buf[count]);
    ++count;
  }
  return count;
}

const char _FullSerialName[] = "FDS:";

_Driver _FullDuplexSerialDriver =
  {
    _FullSerialName,
    fdserial_fopen,
    fdserial_fclose,
    fdserial_read,
    fdserial_write,
    NULL,       /* seek, not needed */
    NULL,       /* remove */
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
