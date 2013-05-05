/*
 * simple serial driver
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cog.h>
#include <errno.h>
#include <propeller.h>
#include <sys/driver.h>

/* globals that the loader may change; these represent the default
 * pins to use
 */
extern unsigned int _rxpin;
extern unsigned int _txpin;
extern unsigned int _baud;

/*
 * we use the following elements of the FILE structure
 * drvarg[0] = rxpin
 * drvarg[1] = txpin
 * drvarg[2] = baud
 * drvarg[3] = bitcycles
 */

/*
 * We need _serial_putbyte to always be fcached so that the timing is
 * OK.
 */
#ifdef __PROPELLER2__
__attribute__((fcache))
static int
_serial_putbyte(int c, FILE *fp)
{
  unsigned int bitcycles = fp->drvarg[3];
  unsigned int txpin = fp->drvarg[1];

  unsigned int waitcycles;
  int i, value;

  /* set output */
  setpin(txpin, 1);

  value = (c | 256) << 1;
  waitcycles = getcnt() + bitcycles;
  for (i = 0; i < 10; i++)
    {
      waitcycles = waitcnt2(waitcycles, bitcycles);
      setpin(txpin, (value & 1) ? 1 : 0);
      value >>= 1;
    }
  // if we turn off DIRA, then some boards (like QuickStart) are left with
  // floating pins and garbage output; if we leave it on, we're left with
  // a high pin and other cogs cannot produce output on it
  // the solution is to use FullDuplexSerialDriver instead on applications
  // with multiple cogs
  //_DIRA &= ~txmask;
  return c;
}
#else
__attribute__((fcache))
static int
_serial_putbyte(int c, FILE *fp)
{
  unsigned int txmask = fp->drvarg[1];
  unsigned int bitcycles = fp->drvarg[3];
  unsigned int waitcycles;
  int i, value;

  /* set output */
  _OUTA |= txmask;
  _DIRA |= txmask;

  value = (c | 256) << 1;
  waitcycles = getcnt() + bitcycles;
  for (i = 0; i < 10; i++)
    {
      waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
      if (value & 1)
        _OUTA |= txmask;
      else
        _OUTA &= ~txmask;
      value >>= 1;
    }
  // if we turn off DIRA, then some boards (like QuickStart) are left with
  // floating pins and garbage output; if we leave it on, we're left with
  // a high pin and other cogs cannot produce output on it
  // the solution is to use FullDuplexSerialDriver instead on applications
  // with multiple cogs
  //_DIRA &= ~txmask;
  return c;
}
#endif

/* and here is getbyte */
/* we need to put it in fcache to get it to work in XMM and CMM modes */
#ifdef __PROPELLER2__
__attribute__((fcache))
static int
_serial_getbyte(FILE *fp)
{
  unsigned int bitcycles = fp->drvarg[3];
  unsigned int rxpin = fp->drvarg[0];
  unsigned int waitcycles;
  int value;
  int i;

  /* wait for a start bit */
  if (fp->_flag & _IONONBLOCK) {
    /* if non-blocking I/O, return immediately if no data */
    if ( 0 != getpin(rxpin) )
      return -1;
  } else {
    while (getpin(rxpin))
      ;
  }
  /* sync for one half bit */
  waitcycles = getcnt() + (bitcycles>>1) + bitcycles;
  value = 0;
  for (i = 0; i < 8; i++) {
    waitcycles = waitcnt2(waitcycles, bitcycles);
    value = (getpin(rxpin) << 7) | (value >> 1);
  }
  /* wait for the line to go high (as it will when the stop bit arrives) */
    while (!getpin(rxpin))
      ;
  return value;
}
#else
__attribute__((fcache))
static int
_serial_getbyte(FILE *fp)
{
  unsigned int rxmask = fp->drvarg[0];
  unsigned int bitcycles = fp->drvarg[3];
  unsigned int waitcycles;
  int value;
  int i;

  /* set input */
  _DIRA &= ~rxmask;

  /* wait for a start bit */
  if (fp->_flag & _IONONBLOCK) {
    /* if non-blocking I/O, return immediately if no data */
    if ( 0 != (_INA & rxmask) )
      return -1;
  } else {
    __builtin_propeller_waitpeq(0, rxmask);
  }
  /* sync for one half bit */
  waitcycles = getcnt() + (bitcycles>>1) + bitcycles;
  value = 0;
  for (i = 0; i < 8; i++) {
    waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
    value = ( (0 != (_INA & rxmask)) << 7) | (value >> 1);
  }
  /* wait for the line to go high (as it will when the stop bit arrives) */
  __builtin_propeller_waitpeq(rxmask, rxmask);
  return value;
}
#endif


/*
 * fopen function
 * does whatever is required to open the file
 * note that the string we get will look like:
 * "baud,rxpin,txpin"
 * if there is no string, use the defaults
 */

static int _serial_fopen(FILE *fp, const char *name, const char *mode)
{
  unsigned int baud = _baud;
  unsigned int txpin = _txpin;
  unsigned int rxpin = _rxpin;
  unsigned int bitcycles;

  if (name && *name) {
    baud = atoi(name);
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
  bitcycles = _clkfreq / baud;

  /* set up the array */
#if defined(__PROPELLER2__)
  fp->drvarg[0] = (rxpin);
  fp->drvarg[1] = (txpin);
#else
  fp->drvarg[0] = (1U<<rxpin);
  fp->drvarg[1] = (1U<<txpin);
#endif
  fp->drvarg[2] = baud;
  fp->drvarg[3] = bitcycles;

  /* mark it as being a terminal */
  fp->_flag |= _IODEV;

  /* all OK */
  return 0;
}

/*
 * work around a bug in the QuickStart hardware; we need to leave
 * pin 31 high on exit or we get garbage output
 */
_DESTRUCTOR
static void SimpleSerialExit(void)
{
#ifndef __PROPELLER2__
  _OUTA |= (1<<_txpin);
  _DIRA |= (1<<_txpin);
#endif
}

/*
 * and the actual driver 
 */

const char _SimpleSerialPrefix[] = "SSER:";

_Driver _SimpleSerialDriver =
  {
    _SimpleSerialPrefix,
    _serial_fopen,
    NULL,       /* fclose hook, not needed */
    _term_read,
    _term_write,
    NULL,       /* seek, not needed */
    NULL,       /* remove, not needed */

    _serial_getbyte,
    _serial_putbyte
  };

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
