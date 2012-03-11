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
