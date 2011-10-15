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
 * We need _serial_tx to always be in HUB memory for speed.
 * Time critical functions like this can't live in external memory.
 */
__attribute__((section(".hubtext")))
#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
__attribute__((optimize("O3")))
#endif
static void
_serial_tx(int value, unsigned int txmask, unsigned int bitcycles)
{
  unsigned int waitcycles;
  int i;

  value = (value | 256) << 1;
  waitcycles = _CNT + bitcycles;
  for (i = 0; i < 10; i++)
    {
      waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
      if (value & 1)
	_OUTA |= txmask;
      else
	_OUTA &= ~txmask;
      value >>= 1;
    }
}

/* here is the write function */
__attribute__((section(".hubtext")))
static int
_serial_write(FILE *fp, unsigned char *buf, int size)
{
    unsigned int txmask = fp->drvarg[1];
    unsigned int bitcycles = fp->drvarg[3];
    int value;
    int count = 0;

    /* set output */
    _OUTA |= txmask;
    _DIRA |= txmask;

    while (count < size)
      {
	value = *buf++;
	_serial_tx(value, txmask, bitcycles);
	count++;
      }
    return count;
}

/* and here is read */
/* we need to optimize with -O2 to get it to work in XMM mode */
__attribute__((section(".hubtext")))
#if defined(__PROPELLER_XMM__)
__attribute__((optimize("O3")))
#endif
static int
_serial_read(FILE *fp, unsigned char *buf, int size)
{
  unsigned int rxmask = fp->drvarg[0];
  unsigned int txmask = fp->drvarg[1];
  unsigned int bitcycles = fp->drvarg[3];
  unsigned int waitcycles;
  int value;
  int i;
  int count = 0;
  int cooked = fp->_flag & _IOCOOKED;
  int unbuffered = fp->_flag & _IONBF;

  /* set input */
  _DIRA &= ~rxmask;

  while (count < size)
    {
      /* wait for a start bit */
      __builtin_propeller_waitpeq(0, rxmask);

      /* sync for one half bit */
      waitcycles = _CNT + (bitcycles>>1) + bitcycles;
      value = 0;
      for (i = 0; i < 8; i++) {
	waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
	value = ( (0 != (_INA & rxmask)) << 7) | (value >> 1);
      }
      __builtin_propeller_waitcnt(waitcycles, bitcycles);
      buf[count++] = value;
      /* do cooked mode processing */
      /* for now this echos the input */
      if (cooked) {
	_OUTA |= txmask;
	_DIRA |= txmask;
	
	/* process backspace */
	if ( (value == '\b' || value == 0x7f) && !unbuffered)
	  {
	    if (count >= 2)
	      {
		count -= 2;
		_serial_tx('\b', txmask, bitcycles);
		_serial_tx(' ', txmask, bitcycles);
		_serial_tx('\b', txmask, bitcycles);
	      }
	    else
	      {
		count = 0;
	      }
	  }
	else
	  _serial_tx(value, txmask, bitcycles);
      }
      /* end of line stops the read */
      if (value == '\n') break;
    }
  return count;
}

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
  fp->drvarg[0] = (1U<<rxpin);
  fp->drvarg[1] = (1U<<txpin);
  fp->drvarg[2] = baud;
  fp->drvarg[3] = bitcycles;

  /* all OK */
  return 0;
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
    _serial_read,
    _serial_write,
    NULL,       /* seek, not needed */
    NULL,       /* remove */
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
