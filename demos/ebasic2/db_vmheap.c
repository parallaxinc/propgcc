/* db_vmheap.c - simple heap manager
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */

#include <string.h>
#include "db_vmheap.h"
#include "db_vmdebug.h"
#include "db_image.h"

/* round to multiple of the word size */
#define WORDMASK        (sizeof(VMVALUE) - 1)
#define WORDSIZE(n)     (((n) + WORDMASK) & ~WORDMASK)

/* element sizes */
static size_t elementSizes[] = {
    sizeof(VMVALUE),    /* ObjTypeIntegerVector */
    sizeof(VMFLOAT),    /* ObjTypeFloatVector */
    sizeof(VMHANDLE),   /* ObjTypeStringVector */
    sizeof(uint8_t),    /* ObjTypeByteVector */
    sizeof(uint8_t),    /* ObjTypeSymbol */
    sizeof(uint8_t),    /* ObjTypeArgument */
    sizeof(uint8_t),    /* ObjTypeType */
    sizeof(uint8_t),    /* ObjTypeString */
    sizeof(uint8_t)     /* ObjTypeCode */
};

/* type names */
static char *typeNames[] = {
    "IntegerVector",
    "FloatVector",
    "StringVector",
    "ByteVector",
    "Symbol",
    "Local",
    "Type",
    "String",
    "Code"
};

/* InitHeap - initialize a heap */
ObjHeap *InitHeap(uint8_t *data, size_t size, int nHandles)
{
    size_t overheadSize, dataSize;
    ObjHeap *heap = (ObjHeap *)data;
    VMHANDLE handle;
    int cnt;
    
    /* get the size of the header plus the handle table */
    overheadSize = sizeof(ObjHeap) + nHandles * sizeof(void *);
     
    /* make sure there is enough space left for object data */
    if (size <= overheadSize)
        return NULL;
    dataSize = size - overheadSize;

    /* setup the heap header */
    data += sizeof(ObjHeap);

    /* setup the handle array */
    heap->handles = (VMHANDLE)(data + dataSize);
    heap->endHandles = heap->handles + nHandles;
    heap->nHandles = nHandles;
    
    /* create the handle free list */
    heap->freeHandles = NULL;
    for (handle = heap->handles, cnt = heap->nHandles; --cnt >= 0; ++handle) {
        *handle = (void *)heap->freeHandles;
        heap->freeHandles = handle;
    }

    /* setup the heap */
    heap->data = heap->free = data;
    
    /* return the newly initialized heap */
    return heap;
}

/* NewSymbol - create a new symbol object */
VMHANDLE NewSymbol(ObjHeap *heap, const char *name, StorageClass storageClass, VMHANDLE type)
{
    size_t size = sizeof(Symbol) + strlen(name);
    VMHANDLE object;
    Symbol *sym;
    
    /* allocate a symbol object */
    if (!(object = ObjAlloc(heap, ObjTypeSymbol, size)))
        return NULL;
        
    /* initialze the symbol */
    sym = GetSymbolPtr(object);
    memset(sym, 0, sizeof(Symbol));
    strcpy(sym->name, name);
    sym->storageClass = storageClass;
    sym->type = type;

    /* return the object */
    return object;
}

/* NewLocal - create a new local symbol object */
VMHANDLE NewLocal(ObjHeap *heap, const char *name, VMHANDLE type, VMVALUE offset)
{
    size_t size = sizeof(Local) + strlen(name);
    VMHANDLE object;
    Local *sym;
    
    /* allocate an argument object */
    if (!(object = ObjAlloc(heap, ObjTypeLocal, size)))
        return NULL;
        
    /* initialze the argument */
    sym = GetLocalPtr(object);
    memset(sym, 0, sizeof(Local));
    strcpy(sym->name, name);
    sym->type = type;
    sym->offset = offset;

    /* return the object */
    return object;
}

/* NewType - create a new type object */
VMHANDLE NewType(ObjHeap *heap, TypeID id)
{
    VMHANDLE object;
    Type *type;
    
    /* allocate a type object */
    if (!(object = ObjAlloc(heap, ObjTypeType, sizeof(Type))))
        return NULL;
        
    /* initialze the type */
    type = (Type *)GetHeapObjPtr(object);
    memset(type, 0, sizeof(Type));
    type->id = id;

    /* return the object */
    return object;
}

/* NewCode - create a new code object */
VMHANDLE NewCode(ObjHeap *heap, size_t size)
{
    return ObjAlloc(heap, ObjTypeCode, size);
}

/* NewString - create a new string object */
VMHANDLE NewString(ObjHeap *heap, size_t size)
{
    return ObjAlloc(heap, ObjTypeString, size);
}

/* StoreIntegerVector - store an integer vector object */
VMHANDLE StoreIntegerVector(ObjHeap *heap, const VMVALUE *buf, size_t size)
{
    VMHANDLE object;
    VMVALUE *p;

    /* allocate an integer vector object */
    if (!(object = ObjAlloc(heap, ObjTypeIntegerVector, size)))
        return NULL;
    
    /* copy the data into the vector object */
    p = GetIntegerVectorBase(object);
    while (size > 0) {
        *p++ = *buf++;
        --size;
    }
    
    /* return the object */
    return object;
}

/* StoreFloatVector - store a float vector object */
VMHANDLE StoreFloatVector(ObjHeap *heap, const VMFLOAT *buf, size_t size)
{
    VMHANDLE object;
    VMFLOAT *p;

    /* allocate a float vector object */
    if (!(object = ObjAlloc(heap, ObjTypeFloatVector, size)))
        return NULL;
    
    /* copy the data into the vector object */
    p = GetFloatVectorBase(object);
    while (size > 0) {
        *p++ = *buf++;
        --size;
    }
    
    /* return the object */
    return object;
}

/* StoreStringVector - store a string vector object */
VMHANDLE StoreStringVector(ObjHeap *heap, const VMHANDLE *buf, size_t size)
{
    VMHANDLE object;
    VMHANDLE *p;

    /* allocate a string vector object */
    if (!(object = ObjAlloc(heap, ObjTypeStringVector, size)))
        return NULL;
    
    /* copy the data into the vector object */
    p = GetStringVectorBase(object);
    while (size > 0) {
        *p++ = *buf++;
        --size;
    }
    
    /* return the object */
    return object;
}

/* StoreByteVector - store a byte vector object */
VMHANDLE StoreByteVector(ObjHeap *heap, ObjType type, const uint8_t *buf, size_t size)
{
    VMHANDLE object;
    uint8_t *p;

    /* allocate byte vector object */
    if (!(object = ObjAlloc(heap, type, size)))
        return NULL;
    
    /* copy the data into the byte vector object */
    p = GetByteVectorBase(object);
    while (size > 0) {
        *p++ = *buf++;
        --size;
    }
    
    /* return the object */
    return object;
}

/* StoreByteVectorData - store data for a previously allocated byte vector object */
int StoreByteVectorData(ObjHeap *heap, VMHANDLE object, const uint8_t *buf, size_t size)
{
    uint8_t *p;

    /* allocate byte vector object */
    if (!ObjRealloc(heap, object, size))
        return VMFALSE;
    
    /* copy the data into the byte vector object */
    p = GetByteVectorBase(object);
    while (size > 0) {
        *p++ = *buf++;
        --size;
    }
    
    /* return the object */
    return VMTRUE;
}

/* ObjAlloc - allocate a new object */
VMHANDLE ObjAlloc(ObjHeap *heap, ObjType type, size_t size)
{
    size_t byteSize = size * elementSizes[type];
    size_t totalSize = sizeof(ObjHdr) + WORDSIZE(byteSize);
    VMHANDLE handle;
    ObjHdr *hdr;

    /* find a free handle */
    if (!(handle = heap->freeHandles))
        return NULL;

    /* remove the handle from the free list */
    heap->freeHandles = (VMHANDLE)*handle;

    /* make sure there's enough space */
    if ((uint8_t *)heap->handles - heap->free < totalSize) {
        CompactHeap(heap);
        if ((uint8_t *)heap->handles - heap->free < totalSize) {
            *handle = (void *)heap->freeHandles;
            heap->freeHandles = handle;
            return NULL;
        }
    }

    /* allocate the next available block */
    hdr = (ObjHdr *)heap->free;
    heap->free += totalSize;
    
    /* store the backpointer and length */
    hdr->handle = handle;
    hdr->type = type;
    hdr->size = size;
    hdr->refCnt = 1;
    
    /* store the pointer from the handle to the heap data */
    *handle = (void *)(hdr + 1);

    /* return successfully */
    return handle;
}

/* ObjRealloc - reallocate space for an object */
int ObjRealloc(ObjHeap *heap, VMHANDLE handle, size_t size)
{
    ObjHdr *hdr = GetHeapObjHdr(handle);
    size_t byteSize = size * elementSizes[hdr->type];
    size_t totalSize = sizeof(ObjHdr) + WORDSIZE(byteSize);
    VMUVALUE refCnt;
    ObjType type;

    /* make sure there's enough space */
    if ((uint8_t *)heap->handles - heap->free < totalSize) {
        CompactHeap(heap);
        if ((uint8_t *)heap->handles - heap->free < totalSize)
            return VMFALSE;
    }
    
    /* get the old type and reference count and free the old data space */
    type = hdr->type;
    refCnt = hdr->refCnt;
    hdr->handle = NULL;

    /* allocate the new space */
    hdr = (ObjHdr *)heap->free;
    heap->free += totalSize;
    
    /* store the backpointer and length */
    hdr->handle = handle;
    hdr->type = type;
    hdr->size = size;
    hdr->refCnt = refCnt;
    
    /* store the pointer from the handle to the heap data */
    *handle = (void *)(hdr + 1);

    /* return successfully */
    return VMTRUE;
}

/* ObjRelease - release a reference to an object */
void ObjRelease(ObjHeap *heap, VMHANDLE handle)
{
    ObjHdr *hdr = GetHeapObjHdr(handle);
    if (--hdr->refCnt == 0 && hdr->handle) {
        hdr->handle = NULL;
        *handle = (void *)heap->freeHandles;
        heap->freeHandles = handle;
        // BUG!!! Need to decrement the reference counts of any embedded handle references
    }
}

/* CompactHeap - compact the heap */
void CompactHeap(ObjHeap *heap)
{
    uint8_t *data, *next, *free;

    /* initialize the heap scan */
    data = next = heap->data;
    free = heap->free;

    /* compact the heap */
    while (data < free) {
        ObjHdr *hdr = (ObjHdr *)data;
        size_t byteSize = hdr->size * elementSizes[hdr->type];
        size_t totalSize = sizeof(ObjHdr) + WORDSIZE(byteSize);
        if (hdr->handle) {
            if (data != next) {
                *hdr->handle = (void *)(next + sizeof(ObjHdr));
                memmove(next, data, totalSize);
            }
            next += totalSize;
        }
        data += totalSize;
    }

    /* store the new free pointer */
    heap->free = next;
}

/* DumpHeap - dump the heap */
void DumpHeap(ObjHeap *heap)
{
    uint8_t *data, *free;

    /* initialize the heap scan */
    data = heap->data;
    free = heap->free;

    /* compact the heap */
    while (data < free) {
        ObjHdr *hdr = (ObjHdr *)data;
        size_t byteSize = hdr->size * elementSizes[hdr->type];
        size_t totalSize = sizeof(ObjHdr) + WORDSIZE(byteSize);
        VM_printf("%p h: %p, c: %d, t: %s, s: %d\n",
                  hdr,
                  hdr->handle,
                  hdr->refCnt,
                  typeNames[hdr->type],
                  hdr->size);
        if (hdr->handle) {
            switch (hdr->type) {
            case ObjTypeIntegerVector:
                break;
            case ObjTypeFloatVector:
                break;
            case ObjTypeStringVector:
                break;
            case ObjTypeByteVector:
                break;
            case ObjTypeSymbol:
            {
                Symbol *sym = GetSymbolPtr(hdr->handle);
                char *p = sym->name;
                VM_printf("    ");
                while (*p) 
                    VM_putchar(*p++);
                VM_putchar('\n');
                break;
            }
            case ObjTypeLocal:
            {
                Local *sym = GetLocalPtr(hdr->handle);
                char *p = sym->name;
                VM_printf("    ");
                while (*p) 
                    VM_putchar(*p++);
                VM_putchar('\n');
                break;
            }
            case ObjTypeType:
                break;
            case ObjTypeString:
            {
                size_t cnt = hdr->size;
                uint8_t *p = GetStringPtr(hdr->handle);
                VM_printf("    \"");
                while (cnt > 0) {
                    VM_putchar(*p++);
                    --cnt;
                }
                VM_printf("\"\n");
                break;
            }
            case ObjTypeCode:
            {
                uint8_t *code = GetCodePtr(hdr->handle);
                DecodeFunction((VMUVALUE)code, code, hdr->size);
                break;
            }
            default:
                break;
            }
        }
        data += totalSize;
    }
}
