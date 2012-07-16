/* db_system.h - global system context
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_SYSTEM_H__
#define __DB_SYSTEM_H__

#include <setjmp.h>
#include "db_types.h"

/* program limits */
#define MAXLINE         128

/* system context */
typedef struct {
    jmp_buf errorTarget;            /* error target */
    int (*getLine)(void *cookie, char *buf, int len, VMVALUE *pLineNumber);
                                    /* function to get a line of input */
    void *getLineCookie;            /* cookie for the getLine function */
    int lineNumber;                 /* current line number */
    uint8_t *freeSpace;             /* base of free space */
    uint8_t *freeNext;              /* next free space available */
    uint8_t *freeTop;               /* top of free space */
    char lineBuf[MAXLINE];          /* current input line */
    char *linePtr;                  /* pointer to the current character */
} System;

System *InitSystem(uint8_t *freeSpace, size_t freeSize);
uint8_t *AllocateFreeSpace(System *sys, size_t size);
int GetLine(System *sys);
void Abort(System *sys, const char *fmt, ...);

void VM_printf(const char *fmt, ...);
void VM_putchar(int ch);

#endif
