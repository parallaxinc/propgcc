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

#include <stdint.h>

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

/* maximum size of an i2c data transfer */
#define I2C_BUFFER_MAX  32

/* i2c state information */
typedef struct {
    uint32_t scl_mask;
    uint32_t sda_mask;
    uint8_t address;
    uint8_t buffer[I2C_BUFFER_MAX];
    int count;
    int index;
} I2C_STATE;

/* i2c functions */
void i2cInit(I2C_STATE *dev, int scl, int sda);
int i2cBegin(I2C_STATE *dev, int address);
int i2cSend(I2C_STATE *dev, int byte);
int i2cEnd(I2C_STATE *dev);
int i2cRequestBuf(I2C_STATE *dev, int address, int count, uint8_t *buf);
int i2cRequest(I2C_STATE *dev, int address, int count);
int i2cReceive(I2C_STATE *dev);

#if defined(__cplusplus)
}
#endif

#endif
