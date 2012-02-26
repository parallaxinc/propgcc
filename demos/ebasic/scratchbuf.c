#include <string.h>
#include "db_compiler.h"

static DATA_SPACE VMVALUE data[SCRATCHBUFSIZE];
static VMVALUE *dataTop = (VMVALUE *)((char *)data + sizeof(data));

int BufWriteWords(int offset, const VMVALUE *buf, int size)
{
    if (data + offset + size > dataTop)
        return VMFALSE;
    memcpy(data + offset, buf, size * sizeof(VMVALUE));
    return VMTRUE;
}

int BufReadWords(int offset, VMVALUE *buf, int size)
{
    if (data + offset + size > dataTop)
        return VMFALSE;
    memcpy(buf, data + offset, size * sizeof(VMVALUE));
    return VMTRUE;
}

