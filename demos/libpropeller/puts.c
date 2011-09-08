#include "stdio.h"

/*
 * GCC will optimize printf to puts if it can, so we need to
 * provide a definition for it
 */
int puts(const char *str)
{
    int c;
    int r = 0;
    while ( 0 != (c = *str++) ) {
        r += _putc(c);
    }
    r += _putc('\n');
    return r;
}

