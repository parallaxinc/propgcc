#include <propeller.h>

class SquareWaveGenerator 
{
		public:
		SquareWaveGenerator (unsigned pin);
		//~SquareWaveGenerator();
		void setFreq(unsigned f);
		void run();
		void stop();
		
		unsigned myPIN;

};