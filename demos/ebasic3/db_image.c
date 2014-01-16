/* db_image.c - compiled image functions
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#include "db_compiler.h"
#include "db_vmdebug.h"

/* InitImageAllocator - initialize the image allocator */
void InitImageAllocator(ParseContext *c)
{
    c->imageDataFree = c->image->imageData;
    c->imageDataTop = (VMVALUE *)((uint8_t *)c->image + c->imageBufferSize);
}

/* StoreVector - store a vector */
VMVALUE StoreVector(ParseContext *c, const VMVALUE *buf, int size)
{
    VMVALUE addr = (VMVALUE)c->imageDataFree;
    if (c->imageDataFree + size > c->imageDataTop)
        ParseError(c, "insufficient image space");
    memcpy(c->imageDataFree, buf, size * sizeof(VMVALUE));
    c->imageDataFree += size;
    return addr;
}

/* StoreBVector - store a byte vector */
VMVALUE StoreBVector(ParseContext *c, const uint8_t *buf, int size)
{
    return StoreVector(c, (VMVALUE *)buf, GetObjSizeInWords(size));
}
