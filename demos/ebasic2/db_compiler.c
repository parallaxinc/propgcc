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

static uint8_t heap_space[8192];

/* InitCompiler - initialize the compiler and create a parse context */
ParseContext *InitCompiler(System *sys, int maxObjects)
{
    ParseContext *c;

    /* allocate and initialize the parse context */
    if (!(c = (ParseContext *)AllocateFreeSpace(sys, sizeof(ParseContext))))
        ParseError(c, "insufficient space for parse context\n");
    memset(c, 0, sizeof(ParseContext));
    c->sys = sys;
    c->freeMark = sys->freeNext;
    
    c->heap = InitHeap(heap_space, sizeof(heap_space), 128);
        
    /* initialize the common types */
    InitCommonType(c, integerType, TYPE_INTEGER);
    InitCommonType(c, integerArrayType, TYPE_ARRAY);
    c->integerArrayType.type.u.arrayInfo.elementType = CommonType(c, integerType);
    InitCommonType(c, byteType, TYPE_BYTE);
    InitCommonType(c, byteArrayType, TYPE_ARRAY);
    c->byteArrayType.type.u.arrayInfo.elementType = CommonType(c, byteType);
    InitCommonType(c, floatType, TYPE_FLOAT);
    InitCommonType(c, floatArrayType, TYPE_ARRAY);
    c->floatArrayType.type.u.arrayInfo.elementType = CommonType(c, floatType);
    InitCommonType(c, stringType,TYPE_STRING);
    InitCommonType(c, stringArrayType, TYPE_ARRAY);
    c->stringArrayType.type.u.arrayInfo.elementType = CommonType(c, stringType);

    /* initialize the global symbol table */
    InitSymbolTable(&c->globals);

    /* add the intrinsic functions */
    AddIntrinsic(c, "ABS",          "i",    FN_ABS);
    AddIntrinsic(c, "RND",          "i",    FN_RND);
    AddIntrinsic(c, "LEFT$",        "s",    FN_LEFT);
    AddIntrinsic(c, "RIGHT$",       "s",    FN_RIGHT);
    AddIntrinsic(c, "MID$",         "s",    FN_MID);
    AddIntrinsic(c, "CHR$",         "s",    FN_CHR);
    AddIntrinsic(c, "STR$",         "s",    FN_STR);
    AddIntrinsic(c, "VAL",          "i",    FN_VAL);
    AddIntrinsic(c, "ASC",          "i",    FN_ASC);
    AddIntrinsic(c, "LEN",          "i",    FN_LEN);
    AddIntrinsic(c, "printStr",     "i",    FN_printStr);
    AddIntrinsic(c, "printInt",     "i",    FN_printInt);
    AddIntrinsic(c, "printTab",     "i",    FN_printTab);
    AddIntrinsic(c, "printNL",      "i",    FN_printNL);
    AddIntrinsic(c, "printFlush",   "i",    FN_printFlush);
#ifdef PROPELLER
    AddIntrinsic(c, "IN",           "i",    FN_IN);
    AddIntrinsic(c, "OUT",          "i",    FN_OUT);
    AddIntrinsic(c, "HIGH",         "i",    FN_HIGH);
    AddIntrinsic(c, "LOW",          "i",    FN_LOW);
    AddIntrinsic(c, "TOGGLE",       "i",    FN_TOGGLE);
    AddIntrinsic(c, "DIR",          "i",    FN_DIR);
    AddIntrinsic(c, "GETDIR",       "i",    FN_GETDIR);
    AddIntrinsic(c, "CNT",          "i",    FN_CNT);
    AddIntrinsic(c, "PAUSE",        "i",    FN_PAUSE);
    AddIntrinsic(c, "PULSEIN",      "i",    FN_PULSEIN);
    AddIntrinsic(c, "PULSEOUT",     "i",    FN_PULSEOUT);
#endif

    /* return the new parse context */
    return c;
}

/* Compile - compile a program */
VMHANDLE Compile(ParseContext *c, int oneStatement)
{
    System *sys = c->sys;
    int prompt = VMFALSE;
    
    /* setup an error target */
    if (setjmp(c->errorTarget) != 0)
        return NULL;

    /* use the rest of the free space for the compiler heap */
    c->nextLocal = sys->freeNext;
    c->heapSize = sys->freeTop - sys->freeNext;

    /* initialize block nesting table */
    c->btop = (Block *)((char *)c->blockBuf + sizeof(c->blockBuf));
    c->bptr = c->blockBuf - 1;

    /* initialize the code staging buffer */
    c->ctop = c->codeBuf + sizeof(c->codeBuf);
    c->cptr = c->codeBuf;

    /* initialize the scanner */
    c->savedToken = 0;
    c->labels = NULL;

    /* start in the main code */
    c->codeType = CODE_TYPE_MAIN;

    /* initialize scanner */
    c->inComment = VMFALSE;
    
    /* get the next line */
    do {
        Token tkn;
        
        /* display a prompt on continuation lines for immediate mode */
        if (oneStatement && prompt)
            VM_printf("  > ");
            
        /* get the next line */
        if (!GetLine(c))
            return NULL;
            
        /* prompt on continuation lines */
        prompt = VMTRUE;
            
        /* parse the statement */
        if ((tkn = GetToken(c)) != T_EOL)
            ParseStatement(c, tkn);
            
    } while (!oneStatement || c->codeType == CODE_TYPE_FUNCTION || c->bptr >= c->blockBuf);
        
    /* end the main code with a halt */
    putcbyte(c, OP_HALT);
    
    /* write the main code */
    StartCode(c, "main", CODE_TYPE_MAIN);
    StoreCode(c);

    /* free up the space the compiler was consuming */
    sys->freeNext = c->freeMark;

    /* return the main code object */
    return c->code;
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
    c->code = NewCode(c->heap, 0);
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

    /* determine the code size */
    codeSize = (int)(c->cptr - c->codeBuf);

#if 0
    VM_printf("%s:\n", c->codeName);
    DecodeFunction(0, c->codeBuf, codeSize);
    DumpLocals(c);
    VM_printf("\n");
#endif

    /* store the vector object */
    StoreByteVectorData(c->heap, c->code, c->codeBuf, codeSize);

    /* empty the local heap */
    c->nextLocal = c->sys->freeNext;
    InitSymbolTable(&c->arguments);
    InitSymbolTable(&c->locals);
    c->labels = NULL;

    /* reset to compile the next code */
    c->codeType = CODE_TYPE_MAIN;
    c->cptr = c->codeBuf;
}

/* AddIntrinsic - add an intrinsic function to the global symbol table */
void AddIntrinsic(ParseContext *c, char *name, char *types, int index)
{
    VMHANDLE symbol, type;
    Type *typ;
    
    /* make the function type */
    type = NewType(c->heap, TYPE_FUNCTION);
    typ = GetTypePtr(type);
    InitSymbolTable(&typ->u.functionInfo.arguments);
    
    /* set the return type */
    switch (*types++) {
    case 'i':
        typ->u.functionInfo.returnType = CommonType(c, integerType);
        break;
    case 's':
        typ->u.functionInfo.returnType = CommonType(c, stringType);
        break;
    default:
        Fatal(c, "unknown return type: %c", types[-1]);
        break;
    }
    
    /* add a global symbol for the intrinsic function */
    symbol = AddGlobal(c, name, SC_CONSTANT, type);
    GetSymbolPtr(symbol)->v.iValue = INTRINSIC_FLAG | index;
}

/* LocalAlloc - allocate memory from the local heap */
void *LocalAlloc(ParseContext *c, size_t size)
{
    System *sys = c->sys;
    void *p;
    size = (size + ALIGN_MASK) & ~ALIGN_MASK;
    if (c->nextLocal + size > sys->freeTop)
        Fatal(c, "insufficient memory");
    p = c->nextLocal;
    c->nextLocal += size;
    return p;
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
