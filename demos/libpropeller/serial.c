/*
 * simple serial driver
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

#include "stdio.h"
#include "time.h"
#include "cog.h"

/*
 * We use a constructor (_serial_init) to start up the serial port
 * automatically, so main doesn't have to remember to do it. We
 * set up with pins 31 and 30, and 115200 baud (sensible defaults for
 * the C3 and spinsim).
 */

/* globals that the loader may change */
int _rxpin = 31;
int _txpin = 30;
int _baud = 115200;
static int _bitcycles;

/*
 * We need _serial_tx to always be in HUB memory for speed.
 * Time critical functions like this can't live in external memory.
 */

int __attribute__((section(".hubtext"))) _serial_tx(int value)
{
    int i;
    int txpin = _txpin;
    int bitcycles = _bitcycles;
    int waitcycles = _CNT + bitcycles;

    value = (value | 256) << 1;
    for (i = 0; i < 10; i++)
    {
        waitcycles = __builtin_waitcnt(waitcycles, bitcycles);
        _OUTA = (value & 1) << txpin;
        value >>= 1;
    }
    return 1;
}

/*
 * initialization function (run automatically at startup)
 */

__attribute__((constructor))
void _serial_init(void)
{
  _bitcycles = CLOCKS_PER_SEC / _baud;
  _OUTA = (1 << _txpin);
  _DIRA = (1 << _txpin);
}

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
