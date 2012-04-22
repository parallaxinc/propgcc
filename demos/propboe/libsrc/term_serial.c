#include <stdio.h>
#include <propeller.h>
#include "term_serial.h"

static int serial_putchar(TERM *term, int ch);

static TERM_OPS ops = {
	serial_putchar,
	NULL
};

TERM *serialTermStart(TERM_SERIAL *serialTerm, FILE *fp)
{
    TERM *term = &serialTerm->term;
    term->ops = &ops;
    serialTerm->fp = fp;
    return term;
}

void serialTermStop(TERM *term)
{
	TERM_SERIAL *serialTerm = (TERM_SERIAL *)term;
	fflush(serialTerm->fp);
}

static int serial_putchar(TERM *term, int ch)
{
	TERM_SERIAL *serialTerm = (TERM_SERIAL *)term;
	return putc(ch, serialTerm->fp);
}

