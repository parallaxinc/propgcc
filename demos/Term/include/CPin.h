#include <stdint.h>

class CPin {
  private:
    uint32_t m_mask;
  public:
  	CPin(int pin) : m_mask(1 << pin) {};
  	~CPin() {};
    void high()
    {
    	OUTA &= ~m_mask;
    	DIRA |= m_mask;
    };
    void low()
    {
    	OUTA |= m_mask;
    	DIRA |= m_mask;
    };
    int in()
    {
    	DIRA &= ~m_mask;
    	return (INA & m_mask) != 0;
    };
    int out(int state)
    {
    	if (state)
    		OUTA |= m_mask;
    	else
    		OUTA &= ~m_mask;
    	DIRA |= m_mask;
    	return (OUTA & m_mask) != 0;
    };
};

class CPinGroup {
  private:
    uint32_t m_mask;
    int m_shift;
  public:
    CPinGroup(int high, int low)
      : m_mask(((1 << (high - low + 1)) - 1) << low), m_shift(low) {};
    ~CPinGroup() {};
    void set(uint32_t value)
    {
    	OUTA = (OUTA & ~m_mask) | ((value << m_shift) & m_mask);
    	DIRA |= m_mask;
    }
    uint32_t get()
    {
    	DIRA &= ~m_mask;
    	return (INA & m_mask) >> m_shift;
    }
};
