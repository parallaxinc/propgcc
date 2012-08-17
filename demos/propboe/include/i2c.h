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

/* forward type declarations */
typedef struct I2C I2C;

/* i2c operations */
typedef struct {
	int (*term)(I2C *dev);
	int (*read)(I2C *dev, int address, uint8_t *buffer, int count);
	int (*write)(I2C *dev, int address, uint8_t *buffer, int count);
} I2C_OPS;

/* i2c state information */
struct I2C {
    I2C_OPS *ops;
    int address;
    uint8_t buffer[1 + I2C_BUFFER_MAX];
    int count;
    int index;
};

typedef struct {
    I2C i2c;
    int cog;
    volatile I2C_MAILBOX mailbox;
} I2C_COGDRIVER;

typedef struct {
    I2C i2c;
    uint32_t scl_mask;
    uint32_t sda_mask;
} I2C_SIMPLE;

/* i2c functions */
I2C *i2cInit(I2C_COGDRIVER *dev, int scl, int sda, int freq);
I2C *simple_i2cInit(I2C_SIMPLE *dev, int scl, int sda);
int i2cTerm(I2C *dev);
int i2cSendBuf(I2C *dev, int address, uint8_t *buffer, int count);
int i2cBegin(I2C *dev, int address);
int i2cAddByte(I2C *dev, int byte);
int i2cSend(I2C *dev);
int i2cRequestBuf(I2C *dev, int address, uint8_t *buf, int count);
int i2cRequest(I2C *dev, int address, int count);
int i2cGetByte(I2C *dev);

#if defined(__cplusplus)
}
#endif

#endif
