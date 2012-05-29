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
#include "i2c.h"

/* we need to reference this symbol to get the driver linked */
extern int _i2c_driver;
int _i2c_driver_loaded = (int)&_i2c_driver;

extern unsigned int _load_start_i2c_drivercog[];

static int cog_i2cClose(I2C *dev);
static int cog_i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop);
static int cog_i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop);

static I2C_OPS cog_i2c_ops = {
    cog_i2cClose,
    cog_i2cRead,
    cog_i2cWrite
};

I2C *i2cOpen(I2C_COGDRIVER *dev, int scl, int sda, int freq)
{
    I2C_INIT init;
    
    // only allow speeds up to 400khz for now
    if (freq > 400000)
        return NULL;
    
    init.scl = scl;
    init.sda = sda;
    init.ticks_per_cycle = CLKFREQ / freq;
    init.mailbox = &dev->mailbox;
    
    dev->mailbox.cmd = I2C_CMD_INIT;
    
    if ((dev->cog = cognew(_load_start_i2c_drivercog, &init)) < 0)
        return NULL;
    
    while (dev->mailbox.cmd != I2C_CMD_IDLE)
        ;
        
    dev->i2c.ops = &cog_i2c_ops;

    return (I2C *)dev;
}

void *i2cGetCogBuffer(void)
{
    return (void *)_load_start_i2c_drivercog;
}

static int cog_i2cClose(I2C *dev)
{
    I2C_COGDRIVER *cdev = (I2C_COGDRIVER *)dev;

    if (cdev->cog < 0)
        return -1;
    cogstop(cdev->cog);

    return 0;
}

static int cog_i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop)
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

static int cog_i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop)
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
