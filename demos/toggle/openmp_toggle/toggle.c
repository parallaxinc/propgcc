/*
This shows an example for creating an LMM program and Makefile.
It simply toggles some pins at various rates.

This version of the toggle demo uses the OpenMP C pragmas to specify
regions of code that should be run in parallel (on different COGs).
Three independent COGs are used to toggle some pins at different
frequencies. The pins to toggle are defined by MASK1, MASK2, and MASK3.

Copyright (c) 2013 Parallax, Inc.
MIT Licensed.

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
+--------------------------------------------------------------------
*/
#include <propeller.h>

#if defined(__PROPELLER2__)
#define CNT getcnt()
#define OUT _PINB
#define DIR  _DIRB
#define MASK1 (0x000000ff)
#define MASK2 (0x0000ff00)
#define MASK3 (0x00ff0000)
#define MASK4 (0xff000000)
#else
#define OUT _OUTA
#define DIR  _DIRA
#define MASK1 (0x03030303)
#define MASK2 (0x0c0c0c0c)
#define MASK3 (0x00303030)
#define MASK4 (0x00c0c0c0)
#endif


#define DELAY1 11000000
#define DELAY2 17000000
#define DELAY3 37000000

#define TIMES 50

// toggle MASK1 at 1Hz
void
task1(void)
{
  int i;
  int delay = DELAY1;
  DIR |= MASK1;
  for(i = 0; i < TIMES; i++) {
    OUT ^= MASK1;
    waitcnt(CNT+delay);
  }
}

// toggle MASK2 at 2Hz
void
task2(void)
{
  int i;
  int delay = DELAY2;
  DIR |= MASK2;
  for(i = 0; i < TIMES; i++) {
    OUT ^= MASK2;
    waitcnt(CNT+delay);
  }
}

// toggle MASK3 at 3 Hz
void
task3(void)
{
  int i;
  int delay = DELAY3;
  DIR |= MASK3;
  for(i = 0; i < TIMES; i++) {
    OUT ^= MASK3;
    waitcnt(CNT+delay);
  }
}

int
main()
{
#pragma omp parallel sections num_threads(3)
  {
#pragma omp section
    task1();
#pragma omp section
    task2();
#pragma omp section
    task3();
  }
  return 0;
}
