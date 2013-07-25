/* i2c.c - i2c functions that don't require a cog driver

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

#ifndef __PROPELLER2__

#include <propeller.h>
#include "i2c.h"

#define I2C_READ        1
#define I2C_WRITE       0

/* set high by allowing the pin to float high, set low by forcing it low */
#define i2c_float_scl_high(dev) (DIRA &= ~(dev)->scl_mask)
#define i2c_set_scl_low(dev)    (DIRA |= (dev)->scl_mask)
#define i2c_float_sda_high(dev) (DIRA &= ~(dev)->sda_mask)
#define i2c_set_sda_low(dev)    (DIRA |= (dev)->sda_mask)

static int simple_i2cClose(I2C *dev);
static int simple_i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop);
static int simple_i2cReadMore(I2C *dev, uint8_t *buffer, int count, int stop);
static int simple_i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop);
static int simple_i2cWriteMore(I2C *dev, uint8_t *buffer, int count, int stop);

static I2C_OPS simple_i2c_ops = {
    simple_i2cClose,
    simple_i2cRead,
    simple_i2cReadMore,
    simple_i2cWrite,
    simple_i2cWriteMore
};

/* local functions */
static void i2cStart(I2C_SIMPLE *dev);
static void i2cStop(I2C_SIMPLE *dev);
static int i2cSendByte(I2C_SIMPLE *dev, uint8_t byte);
static uint8_t i2cReceiveByte(I2C_SIMPLE *dev, int acknowledge);

I2C *simple_i2cOpen(I2C_SIMPLE *dev, int scl, int sda)
{
    uint32_t both_mask;
    dev->i2c.ops = &simple_i2c_ops;
    dev->scl_mask = 1 << scl;
    dev->sda_mask = 1 << sda;
    both_mask = ~(dev->scl_mask | dev->sda_mask);
    DIRA &= both_mask;
    OUTA &= both_mask;
    return (I2C *)dev;
}

static int simple_i2cClose(I2C *dev)
{
    return 0;
}

static int simple_i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop)
{
    i2cStart((I2C_SIMPLE *)dev);
        
    if (i2cSendByte((I2C_SIMPLE *)dev, address | I2C_READ) != 0)
        return -1;
    
    return simple_i2cReadMore(dev, buffer, count, stop);
}

static int simple_i2cReadMore(I2C *dev, uint8_t *buffer, int count, int stop)
{
    int i;
    
    for (i = 0; --count >= 0; ++i) {
        int byte = i2cReceiveByte((I2C_SIMPLE *)dev, count != 0);
        if (byte < 0)
            return -1;
        buffer[i] = byte;
    }
    
    if (stop)
        i2cStop((I2C_SIMPLE *)dev);
    
    return 0;
}

static int simple_i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop)
{
    i2cStart((I2C_SIMPLE *)dev);
    
    if (i2cSendByte((I2C_SIMPLE *)dev, address | I2C_WRITE) != 0)
        return -1;
    
    return simple_i2cWriteMore(dev, buffer, count, stop);
}

static int simple_i2cWriteMore(I2C *dev, uint8_t *buffer, int count, int stop)
{
    int i;
    
    for (i = 0; i < count; ++i)
        if (i2cSendByte((I2C_SIMPLE *)dev, buffer[i]) != 0)
            return -1;
    
    if (stop)
        i2cStop((I2C_SIMPLE *)dev);
    
    return 0;
}

static void i2cStart(I2C_SIMPLE *dev)
{
    i2c_set_sda_low(dev);
    i2c_set_scl_low(dev);
}

static void i2cStop(I2C_SIMPLE *dev)
{
    i2c_float_scl_high(dev);
    i2c_float_sda_high(dev);
}

static int i2cSendByte(I2C_SIMPLE *dev, uint8_t byte)
{
    int count, result;
    
    /* send the byte, high bit first */
    for (count = 8; --count >= 0; ) {
        if (byte & 0x80)
            i2c_float_sda_high(dev);
        else
            i2c_set_sda_low(dev);
        i2c_float_scl_high(dev);
        i2c_set_scl_low(dev);
        byte <<= 1;
    }
    
    /* receive the acknowledgement from the slave */
    i2c_float_sda_high(dev);
    i2c_float_scl_high(dev);
    result = (INA & dev->sda_mask) != 0;
    i2c_set_scl_low(dev);
    i2c_set_sda_low(dev);
    
    return result;
}

static uint8_t i2cReceiveByte(I2C_SIMPLE *dev, int acknowledge)
{
    uint8_t byte = 0;
    int count;
    
    i2c_float_sda_high(dev);
    
    for (count = 8; --count >= 0; ) {
        byte <<= 1;
        i2c_float_scl_high(dev);
        byte |= (INA & dev->sda_mask) ? 1 : 0;
        i2c_set_scl_low(dev);
    }
    
    // acknowledge
    if (acknowledge)
        i2c_set_sda_low(dev);
    else
        i2c_float_sda_high(dev);
    i2c_float_scl_high(dev);
    i2c_set_scl_low(dev);
    i2c_set_sda_low(dev);
    
    return byte;
}

#endif // __PROPELLER2__
