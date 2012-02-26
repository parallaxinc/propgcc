/* db_system.h - global system context
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_SYSTEM_H__
#define __DB_SYSTEM_H__

#include "db_types.h"

/* program limits */
#define MAXLINE         128

/* system context */
typedef struct {
    uint8_t *freeSpace;             /* base of free space */
    uint8_t *freeNext;              /* next free space available */
    uint8_t *freeTop;               /* top of free space */
    uint8_t *freeMark;              /* saved position for RestoreFreeSpace */
    char lineBuf[MAXLINE];          /* current input line */
    char *linePtr;                  /* pointer to the current character */
} System;

System *InitSystem(uint8_t *freeSpace, size_t freeSize);
uint8_t *AllocateFreeSpace(System *sys, size_t size);
void MarkFreeSpace(System *sys);
void RestoreFreeSpace(System *sys);

#endif
