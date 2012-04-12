#ifndef __TERM_SERIAL_H__
#define __TERM_SERIAL_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdio.h>
#include "term_common.h"

typedef struct {
	TERM term;
	FILE *fp;
} TERM_SERIAL;

TERM *serialTerm_start(TERM_SERIAL *serialTerm, FILE *fp);
void serialTerm_stop(TERM *term);

#ifdef __cplusplus
}
#endif

#endif
