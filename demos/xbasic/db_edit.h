#ifndef __EDIT_H__
#define __EDIT_H__

#include "db_compiler.h"

void EditWorkspace(ParseContext *c);

/* edit buffer interface */
void BufInit(void);
int BufAddLineN(int16_t lineNumber, const char *text);
int BufDeleteLineN(int16_t lineNumber);
int BufSeekN(int16_t lineNumber);
int BufGetLine(int16_t *pLineNumber, char *text);

#endif

