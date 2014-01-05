/* db_vmheap.h - definitions for the simple heap manager
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */
 
#ifndef __DB_VMHEAP_H__
#define __DB_VMHEAP_H__
 
#include "db_system.h"

/* object types */
typedef enum {
    ObjTypeIntegerVector,
    ObjTypeStringVector,
    ObjTypeByteVector,
    ObjTypeSymbol,
    ObjTypeLocal,
    ObjTypeType,
    ObjTypeString,
    ObjTypeCode,
    ObjTypeIntrinsic
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
    } u;
} Type;

/* structure used to construct constant type objects */
typedef struct {
    void *data; // this should point to the type field
    ObjHdr hdr; // this should contain a NULL handle
    Type type;
} ConstantType;

typedef void IntrinsicHandler(Interpreter *i);

/* structure used to construct intrinsic function objects */
typedef struct {
    void *data; // this should point to the type field
    ObjHdr hdr; // this should contain a NULL handle
    IntrinsicHandler *handler;
} IntrinsicFunction;

/* initialize a common type field */
#define InitCommonType(c, field, typeid)                \
            (c)->field.type.id = typeid;                \
            (c)->field.data = (void *)&(c)->field.type; \
            (c)->field.hdr.handle = NULL;               \
            (c)->field.hdr.refCnt = 0;                  \
            (c)->field.hdr.type = ObjTypeType;          \
            (c)->field.hdr.size = sizeof(Type);

/* get a handle to one of the common types */
#define CommonType(c, field)        (&(c)->field.data)

/* add an intrinsic function to the symbol table */
#define AddIntrinsic(c, name, id, types)                            \
            {                                                       \
                id##_struct.data = (void *)&id##_struct.handler;    \
                AddIntrinsic1(c, name, types, IntrinsicHandle(id)); \
            }

/* initialize a common type field */
#define DefIntrinsic(name)                                                      \
            IntrinsicHandler fcn_##name;                                        \
            static IntrinsicFunction name##_struct = {                          \
                            NULL,                           /* data */          \
                            {                                                   \
                                NULL,                       /* hdr.handle */    \
                                0,                          /* hdr.refCnt */    \
                                ObjTypeIntrinsic,           /* hdr.type */      \
                                sizeof(IntrinsicHandler *)  /* hdr.size */      \
                            },                                                  \
                            fcn_##name                      /* handler */       \
            }
            
/* get an intrinsic handle */
#define IntrinsicHandle(name)   &name##_struct.data

/* macros to get the base address of an object */
#define GetIntegerVectorBase(h) ((VMVALUE *)GetHeapObjPtr(h))
#define GetStringVectorBase(h)  ((VMHANDLE *)GetHeapObjPtr(h))
#define GetByteVectorBase(h)    ((uint8_t *)GetHeapObjPtr(h))
#define GetSymbolPtr(h)         ((Symbol *)GetHeapObjPtr(h))
#define GetTypePtr(h)           ((Type *)GetHeapObjPtr(h))
#define GetLocalPtr(h)          ((Local *)GetHeapObjPtr(h))
#define GetStringPtr(h)         ((uint8_t *)GetHeapObjPtr(h))
#define GetCodePtr(h)           ((uint8_t *)GetHeapObjPtr(h))
#define GetIntrinsicHandler(h)  (*(IntrinsicHandler **)GetHeapObjPtr(h))

/* macro to get a pointer to an object in the heap */
#define GetHeapObjPtr(h)        (*(h))
#define GetHeapObjHdr(h)        ((ObjHdr *)*(h) - 1)
#define GetHeapObjType(h)       (GetHeapObjHdr(h)->type)
#define GetHeapObjSize(h)       (GetHeapObjHdr(h)->size)
#define ObjAddRef(h)            do {                                            \
                                    if (h)                                      \
                                        ++GetHeapObjHdr(h)->refCnt;             \
                                } while (0)

/* heap structure */
typedef struct {
    System *sys;                    /* system context */
    int nHandles;                   /* number of handles */
    VMHANDLE handles;               /* array of handles */
    VMHANDLE endHandles;            /* end of the array of handles */
    VMHANDLE freeHandles;           /* list of free handles */
    uint8_t *data;                  /* heap data */
    uint8_t *free;                  /* next free heap location */
    SymbolTable globals;            /* global variables and constants */
    ConstantType integerType;       /* integer type */
    ConstantType integerArrayType;  /* integer array type */
    ConstantType byteType;          /* byte type */
    ConstantType byteArrayType;     /* byte array type */
    ConstantType stringType;        /* string type */
    ConstantType stringArrayType;   /* string array type */
    void (*beforeCompact)(void *cookie);
    void (*afterCompact)(void *cookie);
    void *compactCookie;
} ObjHeap;

/* prototypes */
ObjHeap *InitHeap(System *sys, size_t heapSize, int nHandles);
void ResetHeap(ObjHeap *heap);
void InitSymbolTable(SymbolTable *table);
VMHANDLE AddGlobal(ObjHeap *heap, const char *name, StorageClass storageClass, VMHANDLE type);
VMHANDLE FindGlobal(ObjHeap *heap, const char *name);
void DumpGlobals(ObjHeap *heap);
VMHANDLE AddLocal(ObjHeap *heap, SymbolTable *table, const char *name, VMHANDLE type, VMVALUE offset);
VMHANDLE FindLocal(SymbolTable *table, const char *name);
void DumpLocals(SymbolTable *table, const char *tag);
void AddIntrinsic1(ObjHeap *heap, char *name, char *types, VMHANDLE handler);
VMHANDLE NewSymbol(ObjHeap *heap, const char *name, StorageClass storageClass, VMHANDLE type);
VMHANDLE NewLocal(ObjHeap *heap, const char *name, VMHANDLE type, VMVALUE offset);
VMHANDLE NewType(ObjHeap *heap, TypeID id);
int IsHandleType(VMHANDLE type);
VMHANDLE NewCode(ObjHeap *heap, size_t size);
VMHANDLE NewString(ObjHeap *heap, size_t size);
VMHANDLE StoreIntegerVector(ObjHeap *heap, const VMVALUE *buf, size_t size);
VMHANDLE StoreStringVector(ObjHeap *heap, const VMHANDLE *buf, size_t size);
VMHANDLE StoreByteVector(ObjHeap *heap, ObjType type, const uint8_t *buf, size_t size);
int StoreByteVectorData(ObjHeap *heap, VMHANDLE object, const uint8_t *buf, size_t size);
VMHANDLE ObjAlloc(ObjHeap *heap, ObjType type, size_t size);
int ObjRealloc(ObjHeap *heap, VMHANDLE handle, size_t size);
void ObjRelease(ObjHeap *heap, VMHANDLE handle);
void ReleaseCode(ObjHeap *heap, uint8_t *code, size_t size);
void CompactHeap(ObjHeap *heap);
void DumpHeap(ObjHeap *heap);

#endif
