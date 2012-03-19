/* i2c.c - i2c functions

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

#include <propeller.h>
#include "boe.h"

#define I2C_READ        1
#define I2C_WRITE       0

/* set high by allowing the pin to float high, set low by forcing it low */
#define i2c_float_scl_high(dev) (DIRA &= ~(dev)->scl_mask)
#define i2c_set_scl_low(dev)    (DIRA |= (dev)->scl_mask)
#define i2c_float_sda_high(dev) (DIRA &= ~(dev)->sda_mask)
#define i2c_set_sda_low(dev)    (DIRA |= (dev)->sda_mask)

/* local functions */
static void i2cStart(I2C *dev);
static void i2cStop(I2C *dev);
static int i2cSendByte(I2C *dev, uint8_t byte);
static uint8_t i2cReceiveByte(I2C *dev, int acknowledge);

void i2cInit(I2C *dev, int scl, int sda)
{
    uint32_t both_mask;
    dev->scl_mask = 1 << scl;
    dev->sda_mask = 1 << sda;
    both_mask = ~(dev->scl_mask | dev->sda_mask);
    DIRA &= both_mask;
    OUTA &= both_mask;
}

int i2cBegin(I2C *dev, int address)
{
    dev->address = address;
    dev->count = 0;
}

int i2cSend(I2C *dev, int byte)
{
    if (dev->count >= I2C_BUFFER_MAX)
        return -1;
    dev->buffer[dev->count++] = byte;
    return 0;
}

int i2cEnd(I2C *dev)
{
    int i;
    
    i2cStart(dev);
    
    if (i2cSendByte(dev, (dev->address << 1) | I2C_WRITE) != 0)
        return -1;
    
    for (i = 0; i < dev->count; ++i)
        if (i2cSendByte(dev, dev->buffer[i]) != 0)
            return -1;
    
    i2cStop(dev);
    
    return 0;
}

int i2cRequest(I2C *dev, int address, int count)
{
    int remaining, i;
    
    if (count >= I2C_BUFFER_MAX)
        return -1;
        
    i2cStart(dev);
        
    if (i2cSendByte(dev, (address << 1) | I2C_READ) != 0)
        return -1;
    
    for (i = 0, remaining = count; --remaining >= 0; ++i) {
        int byte = i2cReceiveByte(dev, remaining != 0);
        if (byte < 0)
            return -1;
        dev->buffer[i] = byte;
    }
    
    i2cStop(dev);
    
    dev->count = count;
    dev->index = 0;
    
    return 0;
}

int i2cReceive(I2C *dev)
{
    if (dev->index >= dev->count)
        return -1;
    return dev->buffer[dev->index++];
}

static void i2cStart(I2C *dev)
{
    i2c_set_sda_low(dev);
    i2c_set_scl_low(dev);
}

static void i2cStop(I2C *dev)
{
    i2c_float_scl_high(dev);
    i2c_float_sda_high(dev);
}

static int i2cSendByte(I2C *dev, uint8_t byte)
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

static uint8_t i2cReceiveByte(I2C *dev, int acknowledge)
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

