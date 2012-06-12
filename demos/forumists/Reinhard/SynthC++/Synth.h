
#include <propeller.h>

class Synth 
{
		public:
		Synth (char CTR_AB,unsigned Pin,unsigned Freq);
		
		private:
		unsigned fraction (unsigned a, unsigned b, int shift);
		unsigned short findLeadingBitPosition(unsigned aNumber);
};