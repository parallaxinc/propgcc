/* i2c_interface.c - i2c functions

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
#include "i2clib.h"

int i2cInit(I2C_STATE *dev, int scl, int sda)
{
    extern uint8_t i2c_driver_image_array[];
    I2C_INIT init;
    
    init.scl_mask = 1 << scl;
    init.sda_mask = 1 << sda;
    init.mailbox = &dev->mailbox;
    
    dev->mailbox.cmd = I2C_CMD_INIT;
    
    if ((dev->cog = cognew(i2c_driver_image_array, &init)) < 0)
        return -1;
    
    while (dev->mailbox.cmd != I2C_CMD_IDLE)
        ;
        
    return 0;
}

int i2cTerm(I2C_STATE *dev)
{
    if (dev->cog < 0)
        return -1;
    cogstop(dev->cog);
    return 0;
}

int i2cBegin(I2C_STATE *dev, int address)
{
    dev->mailbox.hdr = (address << 1) | I2C_WRITE;
    dev->count = 0;
    return 0;
}

int i2cSend(I2C_STATE *dev, int byte)
{
    if (dev->count > I2C_BUFFER_MAX)
        return -1;
    dev->buffer[dev->count++] = byte;
    return 0;
}

int i2cEnd(I2C_STATE *dev)
{
    dev->mailbox.buffer = dev->buffer;
    dev->mailbox.count = dev->count;
    dev->mailbox.cmd = I2C_CMD_SEND;
    
    while (dev->mailbox.cmd != I2C_CMD_IDLE)
        ;
    
    return dev->mailbox.sts == I2C_OK ? 0 : -1;
}

int i2cRequestBuf(I2C_STATE *dev, int address, int count, uint8_t *buffer)
{
    dev->mailbox.hdr = (address << 1) | I2C_READ;
    dev->mailbox.buffer = buffer;
    dev->mailbox.count = count;
    dev->mailbox.cmd = I2C_CMD_RECEIVE;
    
    while (dev->mailbox.cmd != I2C_CMD_IDLE)
        ;

    return dev->mailbox.sts == I2C_OK ? 0 : -1;
}

int i2cRequest(I2C_STATE *dev, int address, int count)
{
    if (count > I2C_BUFFER_MAX)
        return -1;
        
    dev->mailbox.hdr = (address << 1) | I2C_READ;
    dev->mailbox.buffer = dev->buffer;
    dev->mailbox.count = dev->count;
    dev->mailbox.cmd = I2C_CMD_RECEIVE;
    
    while (dev->mailbox.cmd != I2C_CMD_IDLE)
        ;

    dev->count = count;
    dev->index = 0;
    
    return dev->mailbox.sts == I2C_OK ? 0 : -1;
}

int i2cReceive(I2C_STATE *dev)
{
    if (dev->index >= dev->count)
        return -1;
    return dev->buffer[dev->index++];
}
