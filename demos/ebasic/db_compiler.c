/* db_compiler.c - a simple basic compiler
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include "db_compiler.h"
#include "db_vmdebug.h"
#include "db_vm.h"

/* local function prototypes */
static void PlaceStrings(ParseContext *c);
static uint8_t *AllocateFreeSpace(ParseContext *c, size_t size);

/* InitCompiler - initialize the compiler */
void InitCompiler(ParseContext *c, uint8_t *freeSpace, size_t freeSize)
{
    memset(c, 0, sizeof(ParseContext));
    c->freeSpace = freeSpace;
    c->freeTop = freeSpace + freeSize;
}

/* Compile - compile a program */
int Compile(ParseContext *c, int maxObjects)
{
    Symbol *sym;
    
    /* setup an error target */
    if (setjmp(c->errorTarget) != 0)
        return VMFALSE;

    /* reset the free space allocator */
    c->freeNext = c->freeSpace;

    /* allocate space for the image header */
    if (!(c->image = (ImageHdr *)AllocateFreeSpace(c, sizeof(ImageHdr))))
        ParseError(c, "insufficient space for image header");

    /* allocate space for the object table */
    if (!(c->image->objects = (int16_t *)AllocateFreeSpace(c, maxObjects * sizeof(int16_t))))
        ParseError(c, "insufficient space for object table\n");
    c->image->objectCount = 0;
    c->maxObjects = maxObjects;
    c->image->objectDataSize = 0;

    /* use the rest of the free space for the compiler heap */
    c->nextGlobal = c->freeNext;
    c->nextLocal = c->freeTop;
    c->heapSize = c->freeTop - c->freeNext;
    c->maxHeapUsed = 0;

    /* initialize the global variable count */
    c->image->variableCount = 0;

    /* initialize block nesting table */
    c->btop = (Block *)((char *)c->blockBuf + sizeof(c->blockBuf));
    c->bptr = c->blockBuf - 1;

    /* initialize the code staging buffer */
    c->ctop = c->codeBuf + sizeof(c->codeBuf);
    c->cptr = c->codeBuf;

    /* initialize the string and label tables */
    c->strings = NULL;
    c->labels = NULL;

    /* start in the main code */
    c->codeType = CODE_TYPE_MAIN;

    /* initialize the global symbol table */
    InitSymbolTable(&c->globals);

    /* add the intrinsic functions */
    AddIntrinsic(c, "ABS",             FN_ABS);
    AddIntrinsic(c, "RND",             FN_RND);
    AddIntrinsic(c, "printStr",        FN_printStr);
    AddIntrinsic(c, "printInt",        FN_printInt);
    AddIntrinsic(c, "printTab",        FN_printTab);
    AddIntrinsic(c, "printNL",         FN_printNL);

    /* initialize scanner */
    c->inComment = VMFALSE;
    
    /* get the next line */
    while (GetLine(c)) {
        Token tkn;
        if ((tkn = GetToken(c)) != T_EOL)
            ParseStatement(c, tkn);
    }

    /* end the main code with a halt */
    putcbyte(c, OP_HALT);
    
    /* write the main code */
    StartCode(c, "main", CODE_TYPE_MAIN);
    c->image->mainCode = c->code;
    StoreCode(c);

    /* allocate the global variable table */
    if (!(c->image->variables = (int16_t *)AllocateFreeSpace(c, c->image->variableCount * sizeof(int16_t))))
        ParseError(c, "insufficient space for variable table");

    /* store the initial values of the global variables */
    for (sym = c->globals.head; sym != NULL; sym = sym->next) {
        if (!(sym->value & INTRINSIC_FLAG))
            c->image->variables[sym->value] = sym->initialValue;
    }

    /* allocate space for the object data */
    if (!(c->image->objectData = (int16_t *)AllocateFreeSpace(c, c->image->objectDataSize * sizeof(int16_t))))
        ParseError(c, "insufficient space for object data");
    if (!BufReadWords(0, c->image->objectData, c->image->objectDataSize))
        ParseError(c, "error reading object data");

    {
        int objectTableSize = c->image->objectCount  * sizeof(int16_t);
        int objectDataSize = c->image->objectDataSize  * sizeof(int16_t);
        int dataSize = objectTableSize + objectDataSize + c->image->variableCount * sizeof(int16_t);
#if 0
        DumpSymbols(&c->globals, "symbols");
#endif
        VM_printf("H:%d", c->maxHeapUsed);
        VM_printf(" O:%d", c->image->objectCount);
        VM_printf(" D:%d", objectDataSize);
        VM_printf(" V:%d", c->image->variableCount);
        VM_printf(" T:%d\n", dataSize);
    }

    /* return successfully */
    return VMTRUE;
}

/* StartCode - start a function or method under construction */
void StartCode(ParseContext *c, char *name, CodeType type)
{
    /* all methods must precede the main code */
    if (type != CODE_TYPE_MAIN && c->cptr > c->codeBuf)
        ParseError(c, "subroutines and functions must precede the main code");

    /* don't allow nested functions or subroutines (for now anyway) */
    if (type != CODE_TYPE_MAIN && c->codeType != CODE_TYPE_MAIN)
        ParseError(c, "nested subroutines and functions are not supported");

    /* initialize the code object under construction */
    c->codeName = name;
    InitSymbolTable(&c->arguments);
    InitSymbolTable(&c->locals);
    c->code = NewObject(c);
    c->localOffset = 0;
    c->codeType = type;
    
    /* write the code prolog */
    if (type != CODE_TYPE_MAIN) {
        putcbyte(c, OP_RESERVE);
        putcbyte(c, 0);
    }
}

/* StoreCode - store the function or method under construction */
void StoreCode(ParseContext *c)
{
    int codeSize;

    /* check for unterminated blocks */
    switch (CurrentBlockType(c)) {
    case BLOCK_IF:
    case BLOCK_ELSE:
        ParseError(c, "expecting END IF");
    case BLOCK_FOR:
        ParseError(c, "expecting NEXT");
    case BLOCK_DO:
        ParseError(c, "expecting LOOP");
    case BLOCK_NONE:
        break;
    }

    /* fixup the RESERVE instruction at the start of the code */
    if (c->codeType != CODE_TYPE_MAIN) {
        c->codeBuf[1] = c->localOffset;
        putcbyte(c, OP_RETURN);
    }

    /* make sure all referenced labels were defined */
    CheckLabels(c);

    /* place string literals defined in this function */
    PlaceStrings(c);
    
    /* determine the code size */
    codeSize = (int)(c->cptr - c->codeBuf);

#if 0
    VM_printf("%s:\n", c->codeName);
    DecodeFunction((c->image->objectDataSize + GetObjSizeInWords(sizeof(VectorObjectHdr))) * sizeof(int16_t), c->codeBuf, codeSize);
    DumpSymbols(&c->arguments, "arguments");
    DumpSymbols(&c->locals, "locals");
    VM_printf("\n");
#endif

    /* store the vector object */
    StoreBVectorData(c, c->code, PROTO_CODE, c->codeBuf, codeSize);

    /* empty the local heap */
    c->nextLocal = c->freeTop;
    InitSymbolTable(&c->arguments);
    InitSymbolTable(&c->locals);
    c->labels = NULL;

    /* reset to compile the next code */
    c->codeType = CODE_TYPE_MAIN;
    c->cptr = c->codeBuf;
}

/* PlaceStrings - place any string literals defined in the current function */
static void PlaceStrings(ParseContext *c)
{
    String *str;
    for (str = c->strings; str != NULL; str = str->next) {
        if (str->fixups) {
            str->object = StoreBVector(c, str->value, strlen((char *)str->value) + 1);
            str->placed = VMTRUE;
            fixup(c, str->fixups, str->object);
            str->fixups = 0;
        }
    }
}

/* AllocateFreeSpace - allocate free space */
static uint8_t *AllocateFreeSpace(ParseContext *c, size_t size)
{
    uint8_t *p = c->freeNext;
    size = (size + ALIGN_MASK) & ~ALIGN_MASK;
    if (p + size > c->freeTop)
        return NULL;
    c->freeNext += size;
    return p;
}

/* AddString - add a string to the string table */
String *AddString(ParseContext *c, char *value)
{
    String *str;
    
    /* check to see if the string is already in the table */
    for (str = c->strings; str != NULL; str = str->next)
        if (strcmp(value, (char *)str->value) == 0)
            return str;

    /* allocate the string structure */
    str = (String *)GlobalAlloc(c, sizeof(String) + strlen(value));
    memset(str, 0, sizeof(String));
    strcpy((char *)str->value, value);
    str->next = c->strings;
    c->strings = str;

    /* return the string table entry */
    return str;
}

/* AddStringRef - add a reference to a string in the string table */
int16_t AddStringRef(String *str, int offset)
{
    int link;

    /* handle strings that have already been placed */
    if (str->placed)
        return str->object;

    /* add a new entry to the fixup list */
    link = str->fixups;
    str->fixups = offset;
    return link;
}

/* AddIntrinsic - add an intrinsic function to the global symbol table */
void AddIntrinsic(ParseContext *c, char *name, int index)
{
    AddGlobal(c, name, SC_CONSTANT, INTRINSIC_FLAG | index, 0);
}

/* GlobalAlloc - allocate memory from the global heap */
void *GlobalAlloc(ParseContext *c, size_t size)
{
    void *p;
    if (size & 1) ++size;
    if (c->nextGlobal + size > c->nextLocal)
        Fatal(c, "insufficient memory");
    p = c->nextGlobal;
    c->nextGlobal += size;
    if (c->heapSize - (c->nextLocal - c->nextGlobal) > c->maxHeapUsed)
        c->maxHeapUsed = c->heapSize - (c->nextLocal - c->nextGlobal);
    return p;
}

/* LocalAlloc - allocate memory from the local heap */
void *LocalAlloc(ParseContext *c, size_t size)
{
    if (size & 1) ++size;
    if (c->nextLocal + size < c->nextGlobal)
        Fatal(c, "insufficient memory");
    c->nextLocal -= size;
    if (c->heapSize - (c->nextLocal - c->nextGlobal) > c->maxHeapUsed)
        c->maxHeapUsed = c->heapSize - (c->nextLocal - c->nextGlobal);
    return c->nextLocal;
}

/* VM_printf - formatted print */
void VM_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_vprintf(fmt, ap);
    va_end(ap);
}

/* Fatal - report a fatal error and exit */
void Fatal(ParseContext *c, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_printf("error: ");
    VM_vprintf(fmt, ap);
    VM_putchar('\n');
    va_end(ap);
    longjmp(c->errorTarget, 1);
}
