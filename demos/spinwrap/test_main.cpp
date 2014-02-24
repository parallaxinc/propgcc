#include "test.h"

int main(void)
{
#if 0
    test obj;
    test obj2;

    obj.set_pins(16, 17);
    obj.blink();
    obj.set_pin(18);
    obj.blink();

    obj2.set_pins(20, 21);
    obj2.blink();
    obj2.set_pin(22);
    obj2.blink();
    
    obj.blink();
    obj2.blink();

    while (1)
    	;
#else
    test obj;
    
    obj.set_pins(26, 27);
    while (1)
    	obj.blink();
#endif

    return 0;
}
