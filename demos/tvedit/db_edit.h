#ifndef __EDIT_H__
#define __EDIT_H__

#include "db_system.h"

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define MAX_PROG_NAME   32
#define EDITBUFSIZE	2048

void EditWorkspace(System *sys);

/* edit buffer interface */
void BufInit(void);
int BufAddLineN(int lineNumber, const char *text);
int BufDeleteLineN(int lineNumber);
int BufSeekN(int lineNumber);
int BufGetLineN(int *pLineNumber, char *text);

int BufAddLine(int offset, const char *text);
int BufDeleteLine(int offset);
int BufSeek(int offset);
int BufGetLine(char *text);
int BufGetLineCount(void);

#endif

