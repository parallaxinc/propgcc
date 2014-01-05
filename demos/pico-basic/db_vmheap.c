/* db_vmheap.c - simple heap manager
 *
 * Copyright (c) 2012 by David Michael Betz.  All rights reserved.
 *
 */

#include <string.h>
#include "db_vm.h"
#include "db_image.h"
#include "db_vmdebug.h"

/* round to multiple of the word size */
#define WORDMASK        (sizeof(VMVALUE) - 1)
#define WORDSIZE(n)     (((n) + WORDMASK) & ~WORDMASK)

/* element sizes */
static size_t elementSizes[] = {
    sizeof(VMVALUE),            /* ObjTypeIntegerVector */
    sizeof(VMHANDLE),           /* ObjTypeStringVector */
    sizeof(uint8_t),            /* ObjTypeByteVector */
    sizeof(uint8_t),            /* ObjTypeSymbol */
    sizeof(uint8_t),            /* ObjTypeArgument */
    sizeof(uint8_t),            /* ObjTypeType */
    sizeof(uint8_t),            /* ObjTypeString */
    sizeof(uint8_t),            /* ObjTypeCode */
    sizeof(IntrinsicHandler *)  /* ObjTypeIntrinsic */
};

/* type names */
static char *typeNames[] = {
    "IntegerVector",
    "StringVector",
    "ByteVector",
    "Symbol",
    "Local",
    "Type",
    "String",
    "Code",
    "Intrinsic"
};

DefIntrinsic(abs);
DefIntrinsic(rnd);
DefIntrinsic(left);
DefIntrinsic(right);
DefIntrinsic(mid);
DefIntrinsic(chr);
DefIntrinsic(str);
DefIntrinsic(val);
DefIntrinsic(asc);
DefIntrinsic(len);
DefIntrinsic(printStr);
DefIntrinsic(printInt);
DefIntrinsic(printTab);
DefIntrinsic(printNL);
DefIntrinsic(printFlush);

/* local functions */
static void ObjRelease1(ObjHeap *heap, VMHANDLE stack);
static VMHANDLE DereferenceAndMaybePushObject(VMHANDLE stack, VMHANDLE object);
static VMHANDLE TraceCode(VMHANDLE stack, uint8_t *code, size_t size);

/* InitHeap - initialize a heap */
ObjHeap *InitHeap(System *sys, size_t size, int nHandles)
{
    size_t overheadSize, dataSize;
    ObjHeap *heap;
    uint8_t *data;
    
    /* get the size of the header plus the handle table */
    overheadSize = sizeof(ObjHeap) + nHandles * sizeof(void *);
     
    /* make sure there is enough space left for object data */
    if (size <= overheadSize)
        longjmp(sys->errorTarget, 1);
    dataSize = size - overheadSize;

    /* allocate and initialize the parse context */
    if (!(data = (uint8_t *)AllocateFreeSpace(sys, size)))
        longjmp(sys->errorTarget, 1);
    heap = (ObjHeap *)data;
    heap->sys = sys;
    
    /* setup the heap header */
    data += sizeof(ObjHeap);

    /* setup the handle array */
    heap->handles = (VMHANDLE)(data + dataSize);
    heap->endHandles = heap->handles + nHandles;
    heap->nHandles = nHandles;
    heap->data = data;
    
    /* initialize the heap */
    ResetHeap(heap);
    
    /* return the new heap */
    return heap;
}

/* ResetHeap - reset the heap to its intial state */
void ResetHeap(ObjHeap *heap)
{
    VMHANDLE handle;
    int cnt;
    
    /* create the handle free list */
    heap->freeHandles = NULL;
    for (handle = heap->handles, cnt = heap->nHandles; --cnt >= 0; ++handle) {
        *handle = (void *)heap->freeHandles;
        heap->freeHandles = handle;
    }

    /* setup the heap free space */
    heap->free = heap->data;
    
    /* initialize the global symbol table */
    InitSymbolTable(&heap->globals);

    /* initialize the common types */
    InitCommonType(heap, integerType, TYPE_INTEGER);
    InitCommonType(heap, integerArrayType, TYPE_ARRAY);
    heap->integerArrayType.type.u.arrayInfo.elementType = CommonType(heap, integerType);
    InitCommonType(heap, byteType, TYPE_BYTE);
    InitCommonType(heap, byteArrayType, TYPE_ARRAY);
    heap->byteArrayType.type.u.arrayInfo.elementType = CommonType(heap, byteType);
    InitCommonType(heap, stringType,TYPE_STRING);
    InitCommonType(heap, stringArrayType, TYPE_ARRAY);
    heap->stringArrayType.type.u.arrayInfo.elementType = CommonType(heap, stringType);

    /* add the intrinsic functions */
    AddIntrinsic(heap, "ABS",          abs,        "i=i")
    AddIntrinsic(heap, "RND",          rnd,        "i=i")
    AddIntrinsic(heap, "LEFT$",        left,       "s=si")
    AddIntrinsic(heap, "RIGHT$",       right,      "s=si")
    AddIntrinsic(heap, "MID$",         mid,        "s=sii")
    AddIntrinsic(heap, "CHR$",         chr,        "s=i")
    AddIntrinsic(heap, "STR$",         str,        "s=i")
    AddIntrinsic(heap, "VAL",          val,        "i=s")
    AddIntrinsic(heap, "ASC",          asc,        "i=s")
    AddIntrinsic(heap, "LEN",          len,        "i=s")
    AddIntrinsic(heap, "printStr",     printStr,   "=s")
    AddIntrinsic(heap, "printInt",     printInt,   "=i")
    AddIntrinsic(heap, "printTab",     printTab,   "=")
    AddIntrinsic(heap, "printNL",      printNL,    "=")
    AddIntrinsic(heap, "printFlush",   printFlush, "=")
}

/* InitSymbolTable - initialize a symbol table */
void InitSymbolTable(SymbolTable *table)
{
    table->head = NULL;
    table->tail = NULL;
    table->count = 0;
}

/* AddGlobal - add a global symbol to the symbol table */
VMHANDLE AddGlobal(ObjHeap *heap, const char *name, StorageClass storageClass, VMHANDLE type)
{
    VMHANDLE symbol;
    
    /* allocate the symbol object */
    if (!(symbol = NewSymbol(heap, name, storageClass, type)))
        return NULL;
        
    /* add it to the symbol table */
    if (heap->globals.tail == NULL)
        heap->globals.head = heap->globals.tail = symbol;
    else {
        Symbol *last = GetSymbolPtr(heap->globals.tail);
        heap->globals.tail = symbol;
        last->next = symbol;
    }
    ++heap->globals.count;
    
    /* return the symbol */
    return symbol;
}

/* FindGlobal - find a symbol in the global symbol table */
VMHANDLE FindGlobal(ObjHeap *heap, const char *name)
{
    VMHANDLE symbol = heap->globals.head;
    while (symbol) {
        Symbol *sym = GetSymbolPtr(symbol);
        if (strcasecmp(name, sym->name) == 0)
            return symbol;
        symbol = sym->next;
    }
    return NULL;
}

/* DumpGlobals - dump the global symbol table */
void DumpGlobals(ObjHeap *heap)
{
    VMHANDLE symbol = heap->globals.head;
    if (symbol) {
        VM_printf("Globals:\n");
        while (symbol) {
            Symbol *sym = GetSymbolPtr(symbol);
            VM_printf("  %s %04x\n", sym->name, sym->v.iValue);
            symbol = sym->next;
        }
    }
}

/* AddLocal - add a symbol to a local symbol table */
VMHANDLE AddLocal(ObjHeap *heap, SymbolTable *table, const char *name, VMHANDLE type, VMVALUE offset)
{
    VMHANDLE local;
    
    /* allocate the local symbol object */
    if (!(local = NewLocal(heap, name, type, offset)))
        return NULL;
        
    /* add it to the symbol table */
    if (table->tail == NULL)
        table->head = table->tail = local;
    else {
        Local *last = GetLocalPtr(table->tail);
        table->tail = local;
        last->next = local;
    }
    ++table->count;
    
    /* return the symbol */
    return local;
}

/* FindLocal - find a local symbol in a symbol table */
VMHANDLE FindLocal(SymbolTable *table, const char *name)
{
    VMHANDLE local = table->head;
    while (local) {
        Local *sym = GetLocalPtr(local);
        if (strcasecmp(name, sym->name) == 0)
            return local;
        local = sym->next;
    }
    return NULL;
}

/* DumpLocals - dump a local symbol table */
void DumpLocals(SymbolTable *table, const char *tag)
{
    VMHANDLE symbol;
    if ((symbol = table->head) != NULL) {
        VM_printf("%s:\n", tag);
        while (symbol) {
            Local *sym = GetLocalPtr(symbol);
            VM_printf("  %s %d\n", sym->name, sym->offset);
            symbol = sym->next;
        }
    }
}

/* AddIntrinsic1 - add an intrinsic function to the global symbol table */
void AddIntrinsic1(ObjHeap *heap, char *name, char *types, VMHANDLE handler)
{
    int argumentCount, handleArgumentCount;
    VMHANDLE symbol, type, argType;
    SymbolTable arguments;
    Type *typ;
    
    /* make the function type */
    if (!(type = NewType(heap, TYPE_FUNCTION)))
        longjmp(heap->sys->errorTarget, 1);
    typ = GetTypePtr(type);
    InitSymbolTable(&arguments);
    
    /* set the return type */
    switch (*types++) {
    case 'i':
        typ->u.functionInfo.returnType = CommonType(heap, integerType);
        break;
    case 's':
        typ->u.functionInfo.returnType = CommonType(heap, stringType);
        break;
    case '=':
        typ->u.functionInfo.returnType = NULL;
        goto argumentTypes;
    default:
        longjmp(heap->sys->errorTarget, 1);
    }
    
argumentTypes:
    /* initialize the argument counts */
    argumentCount = handleArgumentCount = 0;
    
    /* add the argument types */
    if (*types++ == '=') {
        while (*types) {
            switch (*types++) {
            case 'i':
                argType = CommonType(heap, integerType);
                ++argumentCount;
                break;
            case 's':
                argType = CommonType(heap, stringType);
                ++handleArgumentCount;
                break;
            default:
                longjmp(heap->sys->errorTarget, 1);
            }
            AddLocal(heap, &arguments, "", argType, 0);
        }
    }

    /* add a global symbol for the intrinsic function */
    if (!(symbol = AddGlobal(heap, name, SC_CONSTANT, type)))
        longjmp(heap->sys->errorTarget, 1);
    GetSymbolPtr(symbol)->v.hValue = handler;

    /* store the argument symbol table */
    typ = GetTypePtr(type);
    typ->u.functionInfo.arguments = arguments;
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

/* IsHandleType - check to see if a type is a heap object */
int IsHandleType(VMHANDLE type)
{
    switch (GetTypePtr(type)->id) {
    case TYPE_STRING:
    case TYPE_ARRAY:
    case TYPE_FUNCTION:
        return VMTRUE;
    default:
        return VMFALSE;
    }
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
void ObjRelease(ObjHeap *heap, VMHANDLE object)
{
    ObjRelease1(heap, DereferenceAndMaybePushObject(NULL, object));
}

/* ReleaseCode - release references in a block of code */
void ReleaseCode(ObjHeap *heap, uint8_t *code, size_t size)
{
    ObjRelease1(heap, TraceCode(NULL, code, size));
}
    
/* ObjRelease1 - release a reference to an object */
static void ObjRelease1(ObjHeap *heap, VMHANDLE stack)
{
    VMHANDLE object;
    
    /* trace objects until the stack is empty */
    while ((object = stack) != NULL) {
        ObjHdr *hdr = GetHeapObjHdr(object);
        
        /* pop the stack */
        stack = hdr->handle;
    
        /* free the object */
        hdr->handle = NULL;
        *object = (void *)heap->freeHandles;
        heap->freeHandles = object;
        
        /* push any embedded object references */
        switch (hdr->type) {
        case ObjTypeStringVector:
        {
            VMHANDLE *base = GetStringVectorBase(object);
            size_t i;
            for (i = 0; i < hdr->size; ++i)
                stack = DereferenceAndMaybePushObject(stack, base[i]);
            break;
        }
        case ObjTypeSymbol:
        {
            Symbol *symbol = GetSymbolPtr(object);
            stack = DereferenceAndMaybePushObject(stack, symbol->next);
            stack = DereferenceAndMaybePushObject(stack, symbol->type);
            if (IsHandleType(symbol->type))
                stack = DereferenceAndMaybePushObject(stack, symbol->v.hValue);
            break;
        }
        case ObjTypeLocal:
        {
            Local *local = GetLocalPtr(object);
            stack = DereferenceAndMaybePushObject(stack, local->next);
            stack = DereferenceAndMaybePushObject(stack, local->type);
            break;
        }
        case ObjTypeType:
        {
            Type *type = GetTypePtr(object);
            switch (type->id) {
            case TYPE_ARRAY:
                stack = DereferenceAndMaybePushObject(stack, type->u.arrayInfo.elementType);
                break;
            case TYPE_FUNCTION:
                stack = DereferenceAndMaybePushObject(stack, type->u.functionInfo.returnType);
                stack = DereferenceAndMaybePushObject(stack, type->u.functionInfo.arguments.head);
                break;
            default:
                /* no internal references */
                break;
            }
            break;
        }
        case ObjTypeCode:
            stack = TraceCode(stack, GetCodePtr(object), hdr->size);
            break;
        default:    /* no internal references */
            /* should never get here */
            break;
        }
    }
}

/* DereferenceAndMaybePushObject - dereference an object and push it onto the trace stack if refCnt goes to zero */
static VMHANDLE DereferenceAndMaybePushObject(VMHANDLE stack, VMHANDLE object)
{
    if (object != NULL) {
        ObjHdr *hdr = GetHeapObjHdr(object);
        if (hdr->handle && --hdr->refCnt == 0) {
            hdr->handle = stack;
            stack = object;
        }
    }
    return stack;
}

/* TraceCode - trace object references in a code object */
static VMHANDLE TraceCode(VMHANDLE stack, uint8_t *code, size_t size)
{
    uint8_t *p = code;
    uint8_t *end = p + size;
    while (p < end) {
        switch (*p++) {
        case OP_HALT:
            break;
        case OP_BRT:
        case OP_BRTSC:
        case OP_BRF:
        case OP_BRFSC:
        case OP_BR:
            p += sizeof(VMVALUE);
            break;
        case OP_NOT:
        case OP_NEG:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_REM:
        case OP_BNOT:
        case OP_BAND:
        case OP_BOR:
        case OP_BXOR:
        case OP_SHL:
        case OP_SHR:
        case OP_LT:
        case OP_LE:
        case OP_EQ:
        case OP_NE:
        case OP_GE:
        case OP_GT:
            break;
        case OP_LIT:
            p += sizeof(VMVALUE);
            break;
        case OP_LREF:
        case OP_LSET:
        case OP_LREFH:
        case OP_LSETH:
            p += 1;
            break;
        case OP_VREF:
        case OP_VSET:
        case OP_VREFH:
        case OP_VSETH:
            break;
        case OP_RESERVE:
        case OP_RETURN:
            p += 2;
        case OP_CALL:
        case OP_DROP:
            break;
        case OP_LITH:
        case OP_GREF:
        case OP_GSET:
        case OP_GREFH:
        case OP_GSETH:
        {
            VMVALUE tmp;
            get_VMVALUE(tmp, *p++);
            stack = DereferenceAndMaybePushObject(stack, (VMHANDLE)tmp);
            break;
        }
        case OP_CAT:
            break;
        default:
            /* should never reach */
            break;
        }
    }
    
    return stack;
}

/* CompactHeap - compact the heap */
void CompactHeap(ObjHeap *heap)
{
    uint8_t *data, *next, *free;
    
    /* call the client's before function */
    if (heap->beforeCompact)
        (*heap->beforeCompact)(heap->compactCookie);

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
                VM_printf("Moving %p to %p\n", data, next);
                *hdr->handle = (void *)(next + sizeof(ObjHdr));
                memmove(next, data, totalSize);
            }
            next += totalSize;
        }
        data += totalSize;
    }

    /* store the new free pointer */
    heap->free = next;

    /* call the client's before function */
    if (heap->afterCompact)
        (*heap->afterCompact)(heap->compactCookie);
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
