/* i2c.h - definitions for a set of simple i2c functions

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

#ifndef __I2C_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "i2c_driver.h"

/* maximum size of an i2c data transfer */
#define I2C_BUFFER_MAX  32

/* i2c state information */
typedef struct {
    int cog;
    volatile I2C_MAILBOX mailbox;
    uint8_t buffer[1 + I2C_BUFFER_MAX];
    int count;
    int index;
} I2C_STATE;

/* i2c functions */
int i2cInit(I2C_STATE *dev, int scl, int sda);
int i2cTerm(I2C_STATE *dev);
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
