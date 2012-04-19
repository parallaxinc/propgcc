/* Pin.h - Pin class to allow access to single pins

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

class Pin {
  private:
    uint32_t m_mask;
  public:
  	Pin(int pin);
  	~Pin();
    void high();
    void low();
    void toggle();
    void input();
    void output();
    void reverse();
    void direction(int dir);
    int getDirection();
    int get();
    int set(int state);
    int pulseIn(int state);
    void pulseOut(int duration);
};

inline Pin::Pin(int pin) : m_mask(1 << pin)
{
}

inline Pin::~Pin()
{
}

inline void Pin::high()
{
    OUTA |= m_mask;
    DIRA |= m_mask;
}

inline void Pin::low()
{
    OUTA &= ~m_mask;
    DIRA |= m_mask;
}

inline void Pin::toggle()
{
    OUTA ^= m_mask;
    DIRA |= m_mask;
}

inline void Pin::input()
{
    DIRA &= ~m_mask;
}

inline void Pin::output()
{
    DIRA |= m_mask;
}

inline void Pin::reverse()
{
    DIRA ^= m_mask;
}

inline void Pin::direction(int dir)
{
    if (dir)
        DIRA |= m_mask;
    else
        DIRA &= ~m_mask;
}

inline int Pin::getDirection()
{
    return (DIRA & m_mask) ? 1 : 0;
}

inline int Pin::get()
{
    DIRA &= ~m_mask;
    return (INA & m_mask) != 0;
}

inline int Pin::set(int state)
{
    if (state)
        OUTA |= m_mask;
    else
        OUTA &= ~m_mask;
    DIRA |= m_mask;
    return (OUTA & m_mask) != 0;
}
