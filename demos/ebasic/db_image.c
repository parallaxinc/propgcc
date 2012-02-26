/* db_image.c - compiled image functions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include "db_compiler.h"
#include "db_vmdebug.h"

/* StoreVector - store a vector object */
VMVALUE StoreVector(ParseContext *c, const VMVALUE *buf, int size)
{
    VectorObjectHdr hdr;
    int hdrSizeInWords = GetObjSizeInWords(sizeof(hdr));
    VMVALUE object;

    /* allocate and initialize the intrinsic function object */
    object = NewObject(c);

    /* store the object offset */
    c->image->objects[object - 1] = c->image->objectDataSize;

    /* write the object header and data */
    hdr.prototype = PROTO_VECTOR;
    hdr.size = size;
    if (!BufWriteWords((VMVALUE *)&hdr, hdrSizeInWords) || !BufWriteWords(buf, size))
        ParseError(c, "insufficient scratch space");

    /* update the object data size */
    c->image->objectDataSize += hdrSizeInWords + size;

    /* return the object */
    return object;
}

/* StoreBVector - store a byte vector object */
VMVALUE StoreBVector(ParseContext *c, const uint8_t *buf, int size)
{
    VMVALUE object;

    /* allocate and initialize the intrinsic function object */
    object = NewObject(c);

    /* store the vector object */
    StoreBVectorData(c, object, PROTO_BVECTOR, buf, size);

    /* return the object */
    return object;
}

/* StoreBVectorData - store data for a byte vector object */
void StoreBVectorData(ParseContext *c, VMVALUE object, VMVALUE proto, const uint8_t *buf, int size)
{
    VectorObjectHdr hdr;
    int hdrSizeInWords = GetObjSizeInWords(sizeof(hdr));
    int dataSizeInWords = GetObjSizeInWords(size);

    /* store the object offset */
    c->image->objects[object - 1] = c->image->objectDataSize;

    /* write the object header and data */
    hdr.prototype = proto;
    hdr.size = size;
    if (!BufWriteWords((VMVALUE *)&hdr, hdrSizeInWords) || !BufWriteWords((VMVALUE *)buf, dataSizeInWords))
        ParseError(c, "insufficient scratch space");

    /* update the object data size */
    c->image->objectDataSize += hdrSizeInWords + dataSizeInWords;
}

/* NewObject - create a new object */
VMVALUE NewObject(ParseContext *c)
{
    VMVALUE object;
    if (c->image->objectCount >= c->maxObjects)
        ParseError(c, "too many objects");
    object = ++c->image->objectCount;
    c->image->objects[object - 1] = 0;
    return object;
}
