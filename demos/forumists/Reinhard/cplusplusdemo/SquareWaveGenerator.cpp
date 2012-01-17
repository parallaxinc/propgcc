#include "SquareWaveGenerator.h"


// C*tor
SquareWaveGenerator::SquareWaveGenerator (unsigned pin)
{
	 myPIN = pin;
	 CTRA = (1<<28)+myPIN;          
	 DIRA |= 1 << myPIN; 
}
// D'tor
//SquareWaveGenerator::~SquareWaveGenerator()
//{}

void SquareWaveGenerator::setFreq (unsigned f)
{
	FRQA = f * 53;
}

void SquareWaveGenerator::stop()
{
	DIRA &= ~(1 << myPIN);
}

void SquareWaveGenerator::run()
{
	DIRA |= (1 << myPIN); 
}
