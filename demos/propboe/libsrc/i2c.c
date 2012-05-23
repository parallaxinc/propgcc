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
#include "i2c.h"

int i2cTerm(I2C *dev)
{
    return (*dev->ops->term)(dev);
}

int i2cSendBuf(I2C *dev, int address, uint8_t *buffer, int count)
{
    return (*dev->ops->write)(dev, address, buffer, count);
}

int i2cBegin(I2C *dev, int address)
{
    dev->address = address;
    dev->count = 0;
    return 0;
}

int i2cAddByte(I2C *dev, int byte)
{
    if (dev->count > I2C_BUFFER_MAX)
        return -1;
    dev->buffer[dev->count++] = byte;
    return 0;
}

int i2cSend(I2C *dev)
{
    return (*dev->ops->write)(dev, dev->address, dev->buffer, dev->count);
}

int i2cRequestBuf(I2C *dev, int address, uint8_t *buffer, int count)
{
    return (*dev->ops->read)(dev, address, buffer, count);
}

int i2cRequest(I2C *dev, int address, int count)
{
    if (count > I2C_BUFFER_MAX)
        return -1;
        
    if ((*dev->ops->read)(dev, address, dev->buffer, count) != 0)
        return -1;

    dev->count = count;
    dev->index = 0;
    
    return 0;
}

int i2cGetByte(I2C *dev)
{
    if (dev->index >= dev->count)
        return -1;
    return dev->buffer[dev->index++];
}
