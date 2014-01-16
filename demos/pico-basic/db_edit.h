#ifndef __DB_EDIT_H__
#define __DB_EDIT_H__

#include "db_system.h"

#define MAX_PROG_NAME   32

typedef void Handler(void *cookie);

typedef struct {
    char *name;
    Handler *handler;
} UserCmd;

void EditWorkspace(System *sys, UserCmd *userCmds, Handler *evalHandler, void *cookie);

/* edit buffer interface */
void BufInit(void);
int BufAddLineN(VMVALUE lineNumber, const char *text);
int BufDeleteLineN(VMVALUE lineNumber);
int BufSeekN(VMVALUE lineNumber);
int BufGetLine(VMVALUE *pLineNumber, char *text);

#endif

