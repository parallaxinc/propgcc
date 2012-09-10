/*
  OpenMP version of the toggle demo
*/

#include <propeller.h>

#define FIRSTPIN 16
#define LASTPIN  24

void toggle(int pin, int delay)
{
  _DIRA |= (1<<pin);
  _OUTA ^= (1<<pin);
  waitcnt(CNT+delay);
}


int
main()
{
  int i;
  int delay = CLKFREQ / 4;
  
#pragma omp parallel
  for(;;) {
#pragma omp for
    for (i = FIRSTPIN; i < LASTPIN; i++) {
      toggle(i, delay);
    }
  }
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
+--------------------------------------------------------------------
*/
