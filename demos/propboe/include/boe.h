/* boe.h - definitions for the PropBOE library

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

#ifndef __BOE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/* pin directions */
#define IN      0
#define OUT     1

/* pin states */
#define HIGH    1
#define LOW     0

/* set pin direction */
void input(int pin);
void output(int pin);

/* get/set pin state */
int getPin(int pin);
void setPin(int pin, int value);
void high(int pin);
void low(int pin);
void toggle(int pin);

/* pulse input/output */
int pulseIn(int pin, int state);
void pulseOut(int pin, int duration);

/* pause for a specified number of millisecons */
void pause(int milliseconds);

#if defined(__cplusplus)
}
#endif

#endif
