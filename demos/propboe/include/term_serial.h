#ifndef __TERM_SERIAL_H__
#define __TERM_SERIAL_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdio.h>
#include "term.h"

typedef struct {
	TERM term;
	FILE *fp;
} TERM_SERIAL;

TERM *serialTermStart(TERM_SERIAL *serialTerm, FILE *fp);
void serialTermStop(TERM *term);

#ifdef __cplusplus
}
#endif

#endif
