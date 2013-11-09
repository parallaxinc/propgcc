#if !defined(__PROPELLER2__)
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

static int cog_i2cClose(I2C *dev);

static I2C_OPS cog_i2c_ops = {
    cog_i2cClose,
    cog_i2cRead,
    cog_i2cReadMore,
    cog_i2cWrite,
    cog_i2cWriteMore
};

I2C *i2cOpen(I2C_COGDRIVER *dev, int scl, int sda, int freq)
{
    use_cog_driver(i2c_driver);
    I2C_INIT init;
    int id;
    
    // only allow speeds up to 400khz for now
    if (freq > 400000)
        return NULL;
    
    init.scl = scl;
    init.sda = sda;
    init.ticks_per_cycle = CLKFREQ / freq;
    init.mailbox = &dev->mailbox;
    
    dev->mailbox.cmd = I2C_CMD_INIT;
    
    if ((id = load_cog_driver(i2c_driver, &init)) < 0)
        return NULL;
    
    dev->cog = id;
    
    while (dev->mailbox.cmd != I2C_CMD_IDLE)
        ;
        
    dev->i2c.ops = &cog_i2c_ops;

    return (I2C *)dev;
}

static int cog_i2cClose(I2C *dev)
{
    I2C_COGDRIVER *cdev = (I2C_COGDRIVER *)dev;
    cogstop(cdev->cog);
    return 0;
}
#endif
