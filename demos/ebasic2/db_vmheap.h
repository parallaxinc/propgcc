/* db_vmheap.h - definitions for the simple heap manager
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */
 
#ifndef __DB_VMHEAP_H__
#define __DB_VMHEAP_H__
 
#include "db_types.h"

/* object types */
typedef enum {
    ObjTypeIntegerVector,
    ObjTypeFloatVector,
    ObjTypeStringVector,
    ObjTypeByteVector,
    ObjTypeSymbol,
    ObjTypeLocal,
    ObjTypeType,
    ObjTypeString,
    ObjTypeCode
} ObjType;

/* object header structure */
typedef struct {
    VMHANDLE handle;
    VMUVALUE refCnt;
    ObjType type;
    size_t size;
} ObjHdr;

/* storage class ids */
typedef enum {
    SC_CONSTANT,
    SC_VARIABLE
} StorageClass;

/* symbol table */
typedef struct {
    VMHANDLE head;
    VMHANDLE tail;
    int count;
} SymbolTable;

/* value union */
typedef union {
    VMVALUE iValue;
    VMFLOAT fValue;
    VMHANDLE hValue;
} Value;

/* symbol structure */
typedef struct {
    VMHANDLE next;
    VMHANDLE type;
    StorageClass storageClass;
    Value v;
    char name[1];
} Symbol;

/* local symbol structure */
typedef struct {
    VMHANDLE next;
    VMHANDLE type;
    VMVALUE offset;
    char name[1];
} Local;

/* type ids */
typedef enum {
    TYPE_UNKNOWN,
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_BYTE,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_FUNCTION
} TypeID;

/* type definition */
typedef struct {
    TypeID  id;
    union {
        struct {
            VMHANDLE elementType;
        } arrayInfo;
        struct {
            VMHANDLE returnType;
            SymbolTable arguments;
        } functionInfo;
    };
} Type;

/* structure used to construct constant types */
typedef struct {
    void *data; // this should point to the hdr field
    ObjHdr hdr; // this should contain a NULL handle
    Type type;
} ConstantType;

/* macros to get the base address of an object */
#define GetIntegerVectorBase(h) ((VMVALUE *)GetHeapObjPtr(h))
#define GetFloatVectorBase(h)   ((VMFLOAT *)GetHeapObjPtr(h))
#define GetStringVectorBase(h)  ((VMHANDLE *)GetHeapObjPtr(h))
#define GetByteVectorBase(h)    ((uint8_t *)GetHeapObjPtr(h))
#define GetSymbolPtr(h)         ((Symbol *)GetHeapObjPtr(h))
#define GetLocalPtr(h)          ((Local *)GetHeapObjPtr(h))
#define GetStringPtr(h)         ((char *)GetHeapObjPtr(h))
#define GetCodePtr(h)           ((uint8_t *)GetHeapObjPtr(h))

/* macro to get a pointer to an object in the heap */
#define GetHeapObjPtr(h)        (*(h))
#define GetHeapObjHdr(h)        ((ObjHdr *)*(h) - 1)
#define GetHeapObjType(h)       (GetHeapObjHdr(h)->type)
#define GetHeapObjSize(h)       (GetHeapObjHdr(h)->size)
#define ObjAddRef(h)            (++GetHeapObjHdr(h)->refCnt)

/* heap structure */
typedef struct {
    int nHandles;           /* number of handles */
    VMHANDLE handles;       /* array of handles */
    VMHANDLE endHandles;    /* end of the array of handles */
    VMHANDLE freeHandles;   /* list of free handles */
    uint8_t *data;          /* heap data */
    uint8_t *free;          /* next free heap location */
} ObjHeap;

/* prototypes */
ObjHeap *InitHeap(uint8_t *data, size_t size, int nHandles);
VMHANDLE NewSymbol(ObjHeap *heap, const char *name, StorageClass storageClass, VMHANDLE type);
VMHANDLE NewLocal(ObjHeap *heap, const char *name, VMHANDLE type, VMVALUE offset);
VMHANDLE NewType(ObjHeap *heap, TypeID id);
VMHANDLE NewCode(ObjHeap *heap, size_t size);
VMHANDLE StoreIntegerVector(ObjHeap *heap, const VMVALUE *buf, size_t size);
VMHANDLE StoreFloatVector(ObjHeap *heap, const VMFLOAT *buf, size_t size);
VMHANDLE StoreStringVector(ObjHeap *heap, const VMHANDLE *buf, size_t size);
VMHANDLE StoreByteVector(ObjHeap *heap, ObjType type, const uint8_t *buf, size_t size);
int StoreByteVectorData(ObjHeap *heap, VMHANDLE object, const uint8_t *buf, size_t size);
VMHANDLE ObjAlloc(ObjHeap *heap, ObjType type, size_t size);
int ObjRealloc(ObjHeap *heap, VMHANDLE handle, size_t size);
void ObjRelease(ObjHeap *heap, VMHANDLE handle);
void CompactHeap(ObjHeap *heap);
void DumpHeap(ObjHeap *heap);

#endif
