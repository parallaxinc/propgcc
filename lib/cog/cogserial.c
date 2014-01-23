#include <stdio.h>
#include <propeller.h>

#define _WaitCnt(a,b) __builtin_propeller_waitcnt(a,b)

/* globals that the loader may change; these represent the default
 * pins to use
 */
#ifdef __PROPELLER2__
unsigned int _rxpin = 91;
unsigned int _txpin = 90;
#else
unsigned int _rxpin = 31;
unsigned int _txpin = 30;
#endif
unsigned int _baud = 115200;
unsigned int _bitcycles;

int putchar(int origval)
{
#if !defined(__PROPELLER2__)
  int txmask = (1<<_txpin);
#endif
  int value = origval;
  int i;
  int bitcycles;
  int waitcycles;

  if (origval == '\n')
    putchar('\r');

#ifdef __PROPELLER2__
  setpin(_txpin, 1);
#else
  _DIRA |= txmask;
  _OUTA |= txmask;
#endif

  if (_bitcycles == 0)
    _bitcycles = _clkfreq / _baud;
  bitcycles = _bitcycles;
  waitcycles = getcnt() + bitcycles;

  value = (value | 256) << 1;
  for (i = 0; i < 10; i++)
    {
      waitcycles = _WaitCnt(waitcycles, bitcycles);
#ifdef __PROPELLER2__
      setpin(_txpin, (value & 1) ? 1 : 0);
#else
      if (value & 1)
	_OUTA |= txmask;
      else
	_OUTA &= ~txmask;
#endif
      value >>= 1;
    }
  return origval;
}
