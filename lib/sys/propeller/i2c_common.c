/* i2c_common.c - common i2c functions between the i2c driver and the i2c boot driver

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
#include "i2c.h"

int cog_i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop)
{
    I2C_COGDRIVER *cdev = (I2C_COGDRIVER *)dev;

    cdev->mailbox.hdr = address | I2C_READ;
    cdev->mailbox.buffer = buffer;
    cdev->mailbox.count = count;
    cdev->mailbox.stop = stop;
    cdev->mailbox.cmd = I2C_CMD_RECEIVE;
    
    while (cdev->mailbox.cmd != I2C_CMD_IDLE)
        ;

    return cdev->mailbox.sts == I2C_OK ? 0 : -1;
}

int cog_i2cReadMore(I2C *dev, uint8_t *buffer, int count, int stop)
{
    I2C_COGDRIVER *cdev = (I2C_COGDRIVER *)dev;

    cdev->mailbox.buffer = buffer;
    cdev->mailbox.count = count;
    cdev->mailbox.stop = stop;
    cdev->mailbox.cmd = I2C_CMD_RECEIVE_MORE;
    
    while (cdev->mailbox.cmd != I2C_CMD_IDLE)
        ;

    return cdev->mailbox.sts == I2C_OK ? 0 : -1;
}

int cog_i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop)
{
    I2C_COGDRIVER *cdev = (I2C_COGDRIVER *)dev;
    
    cdev->mailbox.hdr = address | I2C_WRITE;
    cdev->mailbox.buffer = buffer;
    cdev->mailbox.count = count;
    cdev->mailbox.stop = stop;
    cdev->mailbox.cmd = I2C_CMD_SEND;
    
    while (cdev->mailbox.cmd != I2C_CMD_IDLE)
        ;

    return cdev->mailbox.sts == I2C_OK ? 0 : -1;
}

int cog_i2cWriteMore(I2C *dev, uint8_t *buffer, int count, int stop)
{
    I2C_COGDRIVER *cdev = (I2C_COGDRIVER *)dev;
    
    cdev->mailbox.buffer = buffer;
    cdev->mailbox.count = count;
    cdev->mailbox.stop = stop;
    cdev->mailbox.cmd = I2C_CMD_SEND_MORE;
    
    while (cdev->mailbox.cmd != I2C_CMD_IDLE)
        ;

    return cdev->mailbox.sts == I2C_OK ? 0 : -1;
}
