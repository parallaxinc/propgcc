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
unsigned int _rxpin = 31;
unsigned int _txpin = 30;
unsigned int _baud = 115200;

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

/* here is the putbyte function */
static int __attribute__((section(".hubtext"))) _serial_tx(int value, FILE *fp)
{
    int i;
    unsigned int txpin = fp->drvarg[1];
    unsigned int bitcycles = fp->drvarg[3];
    unsigned int waitcycles = _CNT + bitcycles;
    int orig = value;

    /* set output */
    _OUTA = (1<<txpin);
    _DIRA = (1<<txpin);

    value = (value | 256) << 1;
    for (i = 0; i < 10; i++)
    {
        waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
        _OUTA = (value & 1) << txpin;
        value >>= 1;
    }
    return orig;
}

/* and here is getbyte */
static int __attribute__((section(".hubtext"))) _serial_rx(FILE *fp)
{
  unsigned int rxpin = fp->drvarg[0];
  unsigned int bitcycles = fp->drvarg[3];
  unsigned int waitcycles;
  int value = 0;
  int i;

  int mask = 1<<rxpin;

  /* set input */
  _DIRA &= ~mask;

  /* wait for a start bit */
  __builtin_propeller_waitpeq(0, mask);

  /* sync for one half bit */
  waitcycles = _CNT + (bitcycles>>1) + bitcycles;

  for (i = 0; i < 8; i++) {
    waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
    value = ( (0 != (_INA & mask)) << 7) | (value >> 1);
  }
  __builtin_propeller_waitcnt(waitcycles, bitcycles);

  return value;
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
	txpin = atoi(name);
    }
  }
  bitcycles = _clkfreq / baud;

  /* set up the array */
  fp->drvarg[0] = rxpin;
  fp->drvarg[1] = txpin;
  fp->drvarg[2] = baud;
  fp->drvarg[3] = bitcycles;

  /* all OK */
  return 0;
}

/* send a break on the default transmit pin */

void _serial_break(void)
{
  int delay = _clkfreq/2;
  int waitcycles = _CNT + delay;
  int txpin = _txpin;

  _DIRA |= (1<<txpin);
  _OUTA |= (1<<txpin);

  /* sleep a bit to let things drain */
  waitcycles = __builtin_propeller_waitcnt(waitcycles, delay);

  /* send a break */
  _OUTA = 0;
  __builtin_propeller_waitcnt(waitcycles, delay);
}


/*
 * and the actual driver 
 */

const char _SimpleSerialName[] = "SSER:";

_Driver _SimpleSerialDriver =
  {
    _SimpleSerialName,
    _serial_fopen,
    NULL,       /* fclose hook, not needed */
    NULL,       /* flush hook, not needed */
    _serial_rx,
    _serial_tx,
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
