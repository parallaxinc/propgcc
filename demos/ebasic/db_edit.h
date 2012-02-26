#ifndef __EDIT_H__
#define __EDIT_H__

#include "db_compiler.h"

void EditWorkspace(ParseContext *c);

/* edit buffer interface */
void BufInit(void);
int BufAddLineN(VMVALUE lineNumber, const char *text);
int BufDeleteLineN(VMVALUE lineNumber);
int BufSeekN(VMVALUE lineNumber);
int BufGetLine(VMVALUE *pLineNumber, char *text);

#endif

