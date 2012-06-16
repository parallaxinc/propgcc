/* pin.h - definitions for the pin library

Copyright (c) 2012 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef __PIN_H__
#define __PIN_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

/* pin directions */
#define IN      0
#define OUT     1

/* pin states */
#define HIGH    1
#define LOW     0

/* set pin direction */
void pinInput(int pin);
void pinOutput(int pin);
void pinReverse(int pin);
int pinGetDirection(int pin);
void pinSetDirection(int pin, int direction);

/* get/set pin state */
int pinGet(int pin);
int pinGetField(int high, int low);
void pinSet(int pin, int value);
void pinSetField(int high, int low, int value);
void pinHigh(int pin);
void pinLow(int pin);
void pinToggle(int pin);

/* pulse input/output */
int pinPulseIn(int pin, int state);
void pinPulseOut(int pin, int duration);

#if defined(__cplusplus)
}
#endif

#endif
