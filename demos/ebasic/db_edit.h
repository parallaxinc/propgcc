#ifndef __EDIT_H__
#define __EDIT_H__

#include "db_system.h"

#define MAX_PROG_NAME   32

void EditWorkspace(System *sys);

/* edit buffer interface */
void BufInit(void);
int BufAddLineN(VMVALUE lineNumber, const char *text);
int BufDeleteLineN(VMVALUE lineNumber);
int BufSeekN(VMVALUE lineNumber);
int BufGetLine(VMVALUE *pLineNumber, char *text);

#endif

