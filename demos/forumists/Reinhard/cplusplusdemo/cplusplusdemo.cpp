#include "SquareWaveGenerator.h"

//SquareWaveGenerator * gen;

int main()
{
    SquareWaveGenerator    myGenerator(16);  // Instance a SquareWaveGenerator Object
    
    myGenerator.setFreq(10);              // Do something with the object
    waitcnt((CLKFREQ)+CNT); 
    myGenerator.stop();
    
    myGenerator.setFreq(2000);
    myGenerator.run();
    waitcnt((CLKFREQ)+CNT); 
    myGenerator.stop();
    waitcnt((CLKFREQ)+CNT);
    
    myGenerator.setFreq(5);
    waitcnt((CLKFREQ>>2)+CNT);
    myGenerator.run();
    
    //gen = new SquareWaveGenerator(11);
    
    while(1);
    
    return 0;
}
