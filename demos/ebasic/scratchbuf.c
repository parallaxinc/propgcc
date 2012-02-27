#include <string.h>
#include "db_compiler.h"

static DATA_SPACE VMVALUE data[SCRATCHBUFSIZE];
static VMVALUE *dataTop = (VMVALUE *)((char *)data + sizeof(data));
static VMVALUE *dataPtr = data;

void BufRewind(void)
{
    dataPtr = data;
}

int BufWriteWords(const VMVALUE *buf, int size)
{
    if (dataPtr + size > dataTop)
        return VMFALSE;
    memcpy(dataPtr, buf, size * sizeof(VMVALUE));
    dataPtr += size;
    return VMTRUE;
}

int BufReadWords(VMVALUE *buf, int size)
{
    if (dataPtr + size > dataTop)
        return VMFALSE;
    memcpy(buf, dataPtr, size * sizeof(VMVALUE));
    dataPtr += size;
    return VMTRUE;
}

