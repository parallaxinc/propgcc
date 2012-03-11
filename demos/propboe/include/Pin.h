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
  	Pin(int pin) : m_mask(1 << pin) {};
  	~Pin() {};
    void high()
    {
    	OUTA |= m_mask;
    	DIRA |= m_mask;
    };
    void low()
    {
    	OUTA &= ~m_mask;
    	DIRA |= m_mask;
    };
    void toggle()
    {
    	OUTA ^= m_mask;
    	DIRA |= m_mask;
    };
    void input()
    {
        DIRA &= ~m_mask;
    };
    void output()
    {
        DIRA |= m_mask;
    };
    void direction(int dir)
    {
        if (dir)
            DIRA |= m_mask;
        else
            DIRA &= ~m_mask;
    };
    int get()
    {
    	DIRA &= ~m_mask;
    	return (INA & m_mask) != 0;
    };
    int set(int state)
    {
    	if (state)
    		OUTA |= m_mask;
    	else
    		OUTA &= ~m_mask;
    	DIRA |= m_mask;
    	return (OUTA & m_mask) != 0;
    };
    int pulseIn(int state);
    void pulseOut(int duration);
};
