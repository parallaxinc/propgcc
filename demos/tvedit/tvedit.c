#include <stdio.h>
#include "db_edit.h"
#include "db_system.h"

static uint8_t heap[sizeof(System)];

static int TvGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
{
    int ch;
    while ((ch = getchar()) != '\n' && --len >= 2)
    	*buf++ = ch;
    *buf++ = '\n';
    *buf = '\0';
    
    return VMTRUE;
}

int main(void)
{
    System *sys;
    sys = InitSystem(heap, sizeof(heap));
    sys->getLine = TvGetLine;
    EditWorkspace(sys);
    return 0;
}

