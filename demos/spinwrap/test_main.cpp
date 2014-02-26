#include <stdio.h>
#include "test.h"

#define QUICKSTART
//#define ACTIVITYBOARD

int main(void)
{
#ifdef QUICKSTART
    test obj;
    test obj2;

    obj.set_pins(16, 17);
    obj2.set_pins(20, 21);

    printf("obj: pin %d, other pin %d\n", obj.get_pin(), obj.get_other_pin());
    printf("obj2: pin %d, other pin %d\n", obj2.get_pin(), obj2.get_other_pin());

    obj.start_blinker();
    
    obj.blink();
    obj.set_pin(18);
    obj.blink();

    obj2.blink();
    obj2.set_pin(22);
    obj2.blink();
    
    while (1) {
        obj.blink();
        obj2.blink();
    }

#endif

#ifdef ACTIVITYBOARD
    test obj;
    
    obj.set_pins(26, 27);
    while (1)
        obj.blink();
#endif

    return 0;
}
