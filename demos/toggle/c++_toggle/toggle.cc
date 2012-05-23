/*
This shows an example for creating an LMM program and Makefile.
It simply toggles the pins at 1Hz rate.

Copyright (c) 2011 Parallax, Inc.
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

#include "propeller.h"

// this is a bit more elaborate than required, but
// it shows usage of a basic C++ class
class pinset {
private:
  // these variables are not accessible outside the class
  unsigned int mask;  // which pins are in this set
  unsigned int freq;  // the frequency at which to toggle them
public:
  // here are the public variables and methods

  // the basic constructor
  pinset(unsigned int m, unsigned int f) {
    mask = m;
    freq = f;
    DIRA |= m;
  }

  // the run loop
  void run(void) {
    for(;;) {
      OUTA ^= mask;
      waitcnt(freq+CNT);
    }
  }
};

int main(void)
{
  // create a set of pins to toggle at 2Hz
  pinset allpins(0x3fffffff, CLKFREQ>>1);

  // now go and toggle them, forever
  allpins.run();
}

