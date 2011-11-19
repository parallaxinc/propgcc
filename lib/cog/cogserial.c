#include <stdio.h>
#include <cog.h>

#define _WaitCnt(a,b) __builtin_propeller_waitcnt(a,b)

/* globals that the loader may change; these represent the default
 * pins to use
 */
unsigned int _rxpin = 31;
unsigned int _txpin = 30;
unsigned int _baud = 115200;
unsigned int _bitcycles;

int putchar(int origval)
{
  int value = origval;
  int i;
  int txmask = (1<<_txpin);
  int bitcycles;
  int waitcycles;

  _DIRA |= txmask;
  _OUTA |= txmask;

  if (_bitcycles == 0)
    _bitcycles = _clkfreq / _baud;
  bitcycles = _bitcycles;
  waitcycles = _CNT + bitcycles;

  value = (value | 256) << 1;
  for (i = 0; i < 10; i++)
    {
      waitcycles = _WaitCnt(waitcycles, bitcycles);
      if (value & 1)
	_OUTA |= txmask;
      else
	_OUTA &= ~txmask;
      value >>= 1;
    }
  return origval;
}
