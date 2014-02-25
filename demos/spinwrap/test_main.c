#include "test.h"

#define QUICKSTART
//#define ACTIVITYBOARD

int main(void)
{
#ifdef QUICKSTART
    test obj;
    test obj2;

    init_test(&obj);
    
    test_start_blinker(&obj);
    
    test_set_pins(&obj, 16, 17);
    test_blink(&obj);
    test_set_pin(&obj, 18);
    test_blink(&obj);

	init_test(&obj2);
	
    test_set_pins(&obj2, 20, 21);
    test_blink(&obj2);
    test_set_pin(&obj2, 22);
    test_blink(&obj2);
    
    test_blink(&obj);
    test_blink(&obj2);

    while (1)
    	;
    	
#endif

#ifdef ACTIVITYBOARD
    test obj;
    
    init_test(&obj);
    
    test_set_pins(&obj, 26, 27);
    while (1)
    	test_blink(&obj);
#endif

    return 0;
}
