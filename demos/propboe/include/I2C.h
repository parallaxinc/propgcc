/* I2C.h - I2C bus class

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

#include <stdint.h>
#include <propeller.h>
#include "boe.h"

class I2C {
  private:
    I2C_STATE m_dev;
  public:
  	I2C(int scl, int sda);
  	~I2C();
  	int send(int address, uint8_t *buf, int count);
    int begin(int address);
    int send(int byte);
    int end();
    int request(int address, uint8_t *buf, int count);
    int request(int address, int count);
    int receive();
};

inline I2C::I2C(int scl, int sda)
{
    i2cInit(&m_dev, scl, sda);
}

inline I2C::~I2C()
{
}

inline int I2C::send(int address, uint8_t *buf, int count)
{
    return i2cSendBuf(&m_dev, address, buf, count);
}

inline int I2C::begin(int address)
{
    return i2cBegin(&m_dev, address);
}

inline int I2C::send(int byte)
{
    return i2cSend(&m_dev, byte);
}

inline int I2C::end()
{
    return i2cEnd(&m_dev);
}

inline int I2C::request(int address, uint8_t *buf, int count)
{
    return i2cRequestBuf(&m_dev, address, buf, count);
}

inline int I2C::request(int address, int count)
{
    return i2cRequest(&m_dev, address, count);
}

inline int I2C::receive()
{
    return i2cReceive(&m_dev);
}
