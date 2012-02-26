/* db_system.h - global system context
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */

#include "db_system.h"

/* InitSystem - initialize the compiler */
System *InitSystem(uint8_t *freeSpace, size_t freeSize)
{
    System *sys = (System *)freeSpace;
    if (freeSize < sizeof(System))
        return NULL;
    sys->freeSpace = freeSpace + sizeof(System);
    sys->freeTop = freeSpace + freeSize;
    return sys;
}

/* AllocateFreeSpace - allocate free space */
uint8_t *AllocateFreeSpace(System *sys, size_t size)
{
    uint8_t *p = sys->freeNext;
    size = (size + ALIGN_MASK) & ~ALIGN_MASK;
    if (p + size > sys->freeTop)
        return NULL;
    sys->freeNext += size;
    return p;
}

/* MarkFreeSpace - mark the current position in free space */
void MarkFreeSpace(System *sys)
{
    sys->freeMark = sys->freeNext;
}

/* RestoreFreeSpace - restore to previously marked position */
void RestoreFreeSpace(System *sys)
{
    sys->freeNext = sys->freeMark;
}
