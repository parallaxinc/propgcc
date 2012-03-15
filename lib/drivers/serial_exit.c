/*
 * simple serial driver
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

/*
 * special hook which sends a sequence which propeller-load recognizes
 * as an exit
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

__attribute__((section(".hubtext")))
void _serial_tx(int value)
{
  unsigned int txmask = (1<<_txpin);
  unsigned int bitcycles = _clkfreq / _baud;
  unsigned int waitcycles;
  int i;

  /* set output */
  _OUTA |= txmask;
  _DIRA |= txmask;

  value = (value | 256)<<1;
  /* start with a nice long pulse of high bits */
  waitcycles = _CNT + 10*bitcycles;
  for (i = 0; i < 10; i++)
    {
      waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
      if (value & 1)
	_OUTA |= txmask;
      else
	_OUTA &= ~txmask;
      value = value>>1;
    }
}

__attribute__((section(".hubtext")))
void
_serial_exit(int n)
{
  _serial_tx(0xff);
  _serial_tx(0x00);
  _serial_tx(n & 0xff);
}

#ifdef __GNUC__
void _ExitHook(int) __attribute__ ((alias ("_serial_exit")));
#else
void _ExitHook(int n)
{
  _serial_exit(n);
}
#endif
