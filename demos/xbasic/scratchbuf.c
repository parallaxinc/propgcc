#include <string.h>
#include "db_compiler.h"

static int16_t data[SCRATCHBUFSIZE];
static int16_t *dataTop = (int16_t *)((char *)data + sizeof(data));

int BufWriteWords(int offset, const int16_t *buf, int size)
{
    if (data + offset + size > dataTop)
        return VMFALSE;
    memcpy(data + offset, buf, size * sizeof(int16_t));
    return VMTRUE;
}

int BufReadWords(int offset, int16_t *buf, int size)
{
    if (data + offset + size > dataTop)
        return VMFALSE;
    memcpy(buf, data + offset, size * sizeof(int16_t));
    return VMTRUE;
}

