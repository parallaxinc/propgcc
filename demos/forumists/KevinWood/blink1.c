/* Blinks an LED... works with Propeller Demo Board... */

#include <propeller.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

#ifndef __cplusplus /* needed for compatibility... */
#define bool _Bool
#endif


int pinMode(int pin, bool mode) {
    /* mode: 0 = INPUT, 1 = OUTPUT */
    DIRA = (mode << pin);
    return(DIRA);
}

int digitalWrite(int pin, bool state) {
    /* state: 0 = LOW, 1 = HIGH */
    OUTA = (state << pin);
    return(OUTA);       
}

void pause() {
    waitcnt(CLKFREQ+CNT);
}

int main()
{
    const int pin = 23;
    pinMode(pin, OUTPUT);
    
    while(1) {
        digitalWrite(pin, HIGH);
        pause();
        digitalWrite(pin, LOW);
        pause();
    }
    return 0;
}
