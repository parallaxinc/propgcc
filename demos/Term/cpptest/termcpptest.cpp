#include <propeller.h>
#include "CTerm.h"
#include "CPin.h"

#define C3

int main(void)
{
	CTvTerm tv(12);
	CVgaTerm vga(16);
	CSerialTerm serial(stdout);
#ifdef C3
	CPin vgaEnable(15);
	vgaEnable.low();
#endif

	tv.str("Hello, world! (tv)\n");
	vga.str("Hello, world! (vga)\n");
	serial.str("Hello, world! (serial)\n");

	for (;;)
		;

    return 0;
}
