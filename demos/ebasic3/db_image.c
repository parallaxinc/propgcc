/* db_image.c - compiled image functions
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#include "db_system.h"
#include "db_image.h"

/* AllocateImage - allocate a program image */
ImageHdr *AllocateImage(System *sys, int size)
{
    ImageHdr *image;
    if (!(image = (ImageHdr *)AllocateFreeSpace(sys, size)))
        return NULL;
    image->sys = sys;
    image->size = size;
    InitImage(image);
    return image;
}
        
/* InitImage - initialize an image */
void InitImage(ImageHdr *image)
{
    image->free = image->data;
    image->top = (VMVALUE *)((uint8_t *)image + image->size);
}

/* StoreVector - store a vector */
VMVALUE StoreVector(ImageHdr *image, const VMVALUE *buf, int size)
{
    VMVALUE addr = (VMVALUE)image->free;
    if (image->free + size > image->top)
        return NULL;
    memcpy(image->free, buf, size * sizeof(VMVALUE));
    image->free += size;
    return addr;
}

/* StoreBVector - store a byte vector */
VMVALUE StoreBVector(ImageHdr *image, const uint8_t *buf, int size)
{
    return StoreVector(image, (VMVALUE *)buf, GetObjSizeInWords(size));
}
