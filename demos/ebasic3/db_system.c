/* db_system.h - global system context
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdarg.h>
#include "db_system.h"

/* InitSystem - initialize the compiler */
System *InitSystem(uint8_t *freeSpace, size_t freeSize)
{
    System *sys = (System *)freeSpace;
    if (freeSize < sizeof(System))
        return NULL;
    sys->freeSpace = freeSpace + sizeof(System);
    sys->freeTop = freeSpace + freeSize;
    sys->freeNext = sys->freeSpace;
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

/* GetLine - get the next input line */
int GetLine(System *sys)
{
    if (!(*sys->getLine)(sys->getLineCookie, sys->lineBuf, sizeof(sys->lineBuf), &sys->lineNumber))
        return VMFALSE;
    sys->linePtr = sys->lineBuf;
    return VMTRUE;
}

/* VM_printf - formatted print */
void VM_printf(const char *fmt, ...)
{
    char buf[100], *p = buf;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
    va_end(ap);
}

void Abort(System *sys, const char *fmt, ...)
{
    char buf[100], *p = buf;
    va_list ap;
    va_start(ap, fmt);
    VM_printf("error: ");
    vsnprintf(buf, sizeof(buf), fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
    VM_putchar('\n');
    va_end(ap);
    longjmp(sys->errorTarget, 1);
}

