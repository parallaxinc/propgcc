#include <propeller.h>
#include "CTerm.h"

//#define C3

int main(void)
{
	//CTvTerm tv(12);
	//CVgaTerm vga(16);
	CSerialTerm serial(stdout);

#ifdef C3
    DIRA |= 1<<15;
    OUTA &= ~(1<<15);
#endif

	//tv.str("Hello, world! (tv)");
	//vga.str("Hello, world! (vga)");
	serial.str("Hello, world! (serial)");

	for (;;)
		;

    return 0;
}
